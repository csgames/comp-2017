#include <asm/uaccess.h>
#include <asm/spinlock.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/time.h>

#pragma GCC diagnostic ignored "-Wunknown-pragmas"

#pragma mark User-mode and kernel-mode
#define CSGAMES_NEW_STARTUP 0x100
#define CSGAMES_SET_STARTUP 0x101
#define CSGAMES_GET_PROCESSING_END 0x102
#define CSGAMES_GET_ANGELS_PICK 0x103
#define CSGAMES_SET_ANGELS_PICK 0x104
#define CSGAMES_ENABLE_ANGEL 0x105

static char* g_ioctl_names[] = {
	"CSGAMES_NEW_STARTUP",
	"CSGAMES_SET_STARTUP",
	"CSGAMES_GET_PROCESSING_END",
	"CSGAMES_GET_ANGELS_PICK",
	"CSGAMES_SET_ANGELS_PICK",
	"CSGAMES_ENABLE_ANGEL",
};

#define CSGAMES_MAX_STARTUPS 8
#define CSGAMES_MAX_STARTUP_NAME_SIZE 100 // including NUL terminator
#define CSGAMES_STARTUP_PREDICTED_YEAR_COUNT 16

#define SIGNGEL SIGUSR1

struct startup {
	char name[CSGAMES_MAX_STARTUP_NAME_SIZE];
	unsigned capital_in_dollars;
};

struct startup_net_worth {
	struct startup startup;
	unsigned reserved;
	long worth_by_year[CSGAMES_STARTUP_PREDICTED_YEAR_COUNT];
};

#pragma mark - Kernel-mode declarations
#define CSGAMES_DEVICE_NAME "startup_simulator"
#define NANOS_PER_SEC 1000000000
#define LOG(fmt, ...) printk("STARTUP SIMULATOR: " fmt "\n", ## __VA_ARGS__)

struct startup_block {
	struct startup s;
	struct timespec blocked_until;
	unsigned bytes_fed;
	unsigned random;
};

struct user_timeval {
	long tv_sec;
	long tv_usec;
};

static inline struct startup_block* get_current_startup(void);
static inline int is_before(const struct timespec*, const struct timespec*);

static int csgames_new_startup(struct startup __user*);
static int csgames_set_startup(const char __user*);
static int csgames_get_processing_end(struct user_timeval __user*);
static int csgames_get_angels_pick(char __user*);
static int csgames_set_angels_pick(char __user*);
static int csgames_enable_angel(long);

static int csgames_open(struct inode*, struct file*);
static int csgames_close(struct inode*, struct file*);
static long csgames_ioctl(struct file*, unsigned, unsigned long);
static ssize_t csgames_write(struct file*, const char __user*, size_t, loff_t*);
static ssize_t csgames_read(struct file*, char __user*, size_t, loff_t*);

static int csgames_run_angel(void);

static int __init csgames_init(void);
static void __exit csgames_exit(void);

#pragma mark - Kernel-mode definitions

static unsigned char g_temp_buffer[32768];
static struct startup_block g_startups[CSGAMES_MAX_STARTUPS];
static unsigned g_startup_count;
static unsigned g_current_startup = CSGAMES_MAX_STARTUPS;
static unsigned g_angels_pick = CSGAMES_MAX_STARTUPS;
static unsigned g_refcount;
static unsigned g_angel_state;
static int g_angel_enabled = 1;
static DEFINE_SPINLOCK(g_lock);
static struct task_struct* g_angel_target;

static inline struct startup_block* get_current_startup(void) {
	if (g_current_startup == CSGAMES_MAX_STARTUPS) {
		return NULL;
	}
	return &g_startups[g_current_startup];
}

static inline int is_before(const struct timespec* a, const struct timespec* b) {
	if (a->tv_sec < b->tv_sec) {
		return 1;
	}
	if (a->tv_sec == b->tv_sec) {
		return a->tv_nsec < b->tv_nsec;
	}
	return 0;
}

#pragma mark - ioctl implementations
static int csgames_new_startup(struct startup __user* user_startup) {
	struct startup_block local_startup = {};
	unsigned char zero_pos;
	unsigned i;
	
	if (copy_from_user(&local_startup.s, user_startup, sizeof(local_startup.s)) != 0) {
		return -EFAULT;
	}
	
	for (zero_pos = 0; zero_pos < CSGAMES_MAX_STARTUP_NAME_SIZE; ++zero_pos) {
		if (local_startup.s.name[zero_pos] == 0) {
			break;
		}
	}
	
	if (zero_pos == 0 || zero_pos == CSGAMES_MAX_STARTUP_NAME_SIZE) {
		LOG("Startup name is not null-terminated!");
		return -EINVAL;
	}
	memset(&local_startup.s.name[zero_pos], 0, CSGAMES_MAX_STARTUP_NAME_SIZE - zero_pos);
	
	spin_lock(&g_lock);
	if (g_startup_count < CSGAMES_MAX_STARTUPS) {
		for (i = 0; i < g_startup_count; ++i) {
			if (strncmp(local_startup.s.name, g_startups[i].s.name, sizeof(local_startup.s.name)) == 0) {
				break;
			}
		}
		if (i != g_startup_count) {
			spin_unlock(&g_lock);
			
			LOG("A startup already exists with name \"%.100s\"!", local_startup.s.name);
			return -EINVAL;
		}
		
		g_startups[g_startup_count] = local_startup;
		g_current_startup = g_startup_count;
		++g_startup_count;
		spin_unlock(&g_lock);
		
		LOG("Startup \"%.100s\" created.", local_startup.s.name);
		return 0;
	} else {
		spin_unlock(&g_lock);
		
		LOG("Reached maximum number of startups under processing (%u)!", CSGAMES_MAX_STARTUP_NAME_SIZE);
		return -ENFILE;
	}
}

static int csgames_set_startup(const char __user* name) {
	char local_name[CSGAMES_MAX_STARTUP_NAME_SIZE];
	struct timespec now;
	unsigned i;
	unsigned end;
	
	if (copy_from_user(&local_name, (void*)name, sizeof(local_name)) == CSGAMES_MAX_STARTUP_NAME_SIZE) {
		return -EFAULT;
	}

	spin_lock(&g_lock);
	end = g_startup_count;
	for (i = 0; i < end; ++i) {
		if (strncmp(local_name, g_startups[i].s.name, sizeof(local_name)) == 0) {
			break;
		}
	}
	if (i == end) {
		spin_unlock(&g_lock);
		LOG("Couldn't find startup \"%.100s\"!", local_name);
		return -ENOENT;
	}
	
	// If we have a startup, sleep until secs+1 to try to cause a race condition.
	getnstimeofday(&now);
	if (g_startups[i].blocked_until.tv_sec == now.tv_sec) {
		spin_unlock(&g_lock);
		schedule_timeout_interruptible((NANOS_PER_SEC - now.tv_nsec) * HZ / NANOS_PER_SEC);
		
		if (signal_pending(current)) {
			spin_unlock(&g_lock);
			return -EINTR;
		}
				
		// Now try to get the startup again.
		spin_lock(&g_lock);
		end = g_startup_count;
		for (i = 0; i < end; ++i) {
			if (strncmp(local_name, g_startups[i].s.name, sizeof(local_name)) == 0) {
				break;
			}
		}
		if (i == end) {
			spin_unlock(&g_lock);
			LOG("Couldn't find startup \"%.100s\"!", local_name);
			return -ENOENT;
		}
	}
	g_current_startup = i;
	spin_unlock(&g_lock);
	
	LOG("Found and selected startup \"%.100s\".", local_name);
	return 0;
}

static int csgames_get_processing_end(struct user_timeval __user* tv) {
	struct startup_block* startup;
	struct user_timeval ktv;
	
	spin_lock(&g_lock);
	startup = get_current_startup();
	if (startup == NULL)
	{
		spin_unlock(&g_lock);
		
		LOG("No startup selected!");
		return -EINVAL;
	}
	
	ktv.tv_sec = startup->blocked_until.tv_sec;
	ktv.tv_usec = startup->blocked_until.tv_nsec / 1000;
	spin_unlock(&g_lock);
	
	LOG("Returning %li.%06li to user.", ktv.tv_sec, ktv.tv_usec);
	if (copy_to_user(tv, &ktv, sizeof(ktv)) != 0)
	{
		return -EFAULT;
	}
	return 0;
}

static int csgames_get_angels_pick(char __user* name_dest) {
	char startup_name[CSGAMES_MAX_STARTUP_NAME_SIZE];
	int name_length;
	
	spin_lock(&g_lock);
	g_current_startup = CSGAMES_MAX_STARTUPS;
	
	if (g_angels_pick == CSGAMES_MAX_STARTUPS) {
		spin_unlock(&g_lock);
		
		LOG("Angel investor hasn't chosen any startup!");
		return -EAGAIN;
	}
	
	strncpy(startup_name, g_startups[g_angels_pick].s.name, sizeof(startup_name));
	g_startups[g_angels_pick] = g_startups[g_startup_count-1];
	g_angels_pick = CSGAMES_MAX_STARTUPS;
	--g_startup_count;
	spin_unlock(&g_lock);
	
	name_length = strnlen(startup_name, sizeof(startup_name));
	if (copy_to_user(name_dest, startup_name, name_length + 1) != 0) {
		LOG("Failed to copy \"%.100s\" back to user %u! Startup lost!", startup_name, current->pid);
		return -EFAULT;
	}
	
	return 0;
}

static int csgames_set_angels_pick(char __user* name) {
	char local_name[CSGAMES_MAX_STARTUP_NAME_SIZE];
	int i;
	int task_signalled = 0;
	
	if (current_euid().val != 0) {
		return -EPERM;
	}
	
	if (copy_from_user(local_name, name, sizeof(local_name)) != 0) {
		return -EFAULT;
	}
	
	spin_lock(&g_lock);
	if (g_angels_pick == CSGAMES_MAX_STARTUPS) {
		for (i = 0; i < g_startup_count; ++i) {
			if (strncmp(local_name, g_startups[i].s.name, sizeof(local_name)) == 0) {
				break;
			}
		}
		if (i == g_startup_count) {
			spin_unlock(&g_lock);
			LOG("Couldn't find startup \"%.100s\"!", local_name);
			return -ENOENT;
		}
		
		g_angels_pick = i;
		if (g_angel_target != NULL) {
			task_signalled = g_angel_target->pid;
			send_sig(SIGNGEL, g_angel_target, 0);
		}
		spin_unlock(&g_lock);

		if (task_signalled) {
			LOG("*** Signalled process %u of angel investment in startup \"%.100s\".", task_signalled, local_name);
		}
	} else {
		spin_unlock(&g_lock);
		LOG("Another startup has already been picked by the angel investor!");
	}
	
	return 0;
}

static int csgames_enable_angel(long onOrOff) {
	if (current_euid().val != 0) {
		return -EPERM;
	}
	
	spin_lock(&g_lock);
	g_angel_enabled = !!onOrOff;
	spin_unlock(&g_lock);
	
	LOG("Angel investor simulation turned %s", onOrOff ? "on" : "off");
	return 0;
}

#pragma mark - Device operations
static int csgames_open(struct inode* i, struct file* f) {
	int initial_open = 0;
	
	LOG("/dev/" CSGAMES_DEVICE_NAME " has been opened by process %u.", current->pid);
	
	spin_lock(&g_lock);
	++g_refcount;
	if (g_angel_target == NULL) {
		initial_open = 1;
		g_angel_target = current;
	}
	spin_unlock(&g_lock);
	
	if (initial_open) {
		LOG("Angel signals will be delievered to that process.");
	}
	return 0;
}

static int csgames_close(struct inode* i, struct file* f) {
	int initial_close = 0;
	int closed;
	
	LOG("/dev/" CSGAMES_DEVICE_NAME " has been closed by process %u.", current->pid);
	
	spin_lock(&g_lock);
	if (g_angel_target == current) {
		g_angel_target = NULL;
		initial_close = 1;
	}
	--g_refcount;
	closed = g_refcount == 0;
	if (closed) {
		g_startup_count = 0;
		g_current_startup = CSGAMES_MAX_STARTUPS;
		g_angel_target = NULL;
		g_angel_state = 1;
	}
	spin_unlock(&g_lock);
	
	if (initial_close) {
		LOG("Angel signals will no longer be delivered.");
	}
	if (closed) {
		LOG("Device state cleared.");
	}
	return 0;
}

static long csgames_ioctl(struct file* file, unsigned int code, unsigned long value) {
	unsigned int offset = code - 0x100;
	if (offset < sizeof(g_ioctl_names) / sizeof(*g_ioctl_names)) {
		if (csgames_run_angel()) {
			return -EINTR;
		}
		
		LOG("ioctl %u (%s), %p", code, g_ioctl_names[offset], (void*)value);
		switch (code) {
		case CSGAMES_NEW_STARTUP:
			return csgames_new_startup((struct startup __user*)value);
		case CSGAMES_SET_STARTUP:
			return csgames_set_startup((const char __user*)value);
		case CSGAMES_GET_PROCESSING_END:
			return csgames_get_processing_end((struct user_timeval __user*)value);
		case CSGAMES_GET_ANGELS_PICK:
			return csgames_get_angels_pick((char __user*)value);
		case CSGAMES_SET_ANGELS_PICK:
			return csgames_set_angels_pick((char __user*)value);
		case CSGAMES_ENABLE_ANGEL:
			return csgames_enable_angel(value);
		}
	}
	LOG("Unknown ioctl code 0x%x!", code);
	return -EINVAL;
}

static const char g_pdf_magic[] = { '%', 'P', 'D', 'F' };
static ssize_t csgames_write(struct file* file, const char __user* buf, size_t count, loff_t* ppos) {
	char name[CSGAMES_MAX_STARTUP_NAME_SIZE];
	struct timespec now, timeout;
	struct startup_block* startup;
	unsigned random_pseudorandom;
	unsigned initial_value;
	unsigned shift_out;
	int i;
	unsigned short max_bytes_count = 4;
	
	if (csgames_run_angel()) {
		return -EINTR;
	}
	
	getnstimeofday(&now);
	
	spin_lock(&g_lock);
	startup = get_current_startup();
	if (startup == NULL) {
		spin_unlock(&g_lock);
		
		LOG("No startup selected!");
		return -EINVAL;
	}
	
	if (startup == &g_startups[g_angels_pick]) {
		spin_unlock(&g_lock);
		
		LOG("Trying to feed data to startup selected by angel investor!");
		return -EPERM;
	}
	
	random_pseudorandom = startup->random;
	for (i = 0; i < 8; ++i) {
		max_bytes_count += 1 << ((random_pseudorandom + 1) % 11);
		max_bytes_count += 1 << ((random_pseudorandom + 3) % 11);
		max_bytes_count += 1 << ((random_pseudorandom + 7) % 11);
		max_bytes_count += 1 << (random_pseudorandom % 11);
		random_pseudorandom >>= 6;
	}
	if (max_bytes_count > count) {
		max_bytes_count = count;
	}
	if (max_bytes_count > sizeof(g_temp_buffer)) {
		max_bytes_count = sizeof(g_temp_buffer);
	}
	
	if (copy_from_user(g_temp_buffer, buf, max_bytes_count) != 0) {
		spin_unlock(&g_lock);
		LOG("Failed to read %i bytes from user!", max_bytes_count);
		return -EFAULT;
	}
	
	if (startup->bytes_fed == 0) {
		if (max_bytes_count < 4) {
			spin_unlock(&g_lock);
			
			LOG("Initial chunk must be at least 4 bytes large!");
			return -EINVAL;
		}
		
		if (memcmp(g_temp_buffer, g_pdf_magic, sizeof(g_pdf_magic)) != 0) {
			spin_unlock(&g_lock);
			
			LOG("Document is not a PDF file!");
			return -EINVAL;
		}
	}
	
	strncpy(name, startup->s.name, sizeof(name));
	if (is_before(&now, &startup->blocked_until)) {
		++startup->blocked_until.tv_sec;
		timeout = startup->blocked_until;
		spin_unlock(&g_lock);
		
		LOG("Startup \"%.100s\" blocked until %li.%09li!", name, timeout.tv_sec, timeout.tv_nsec);
		return -EAGAIN;
	}
	
	for (i = 0; i < max_bytes_count; ++i) {
		initial_value = startup->random;
		shift_out = startup->random >> 26;
		startup->random <<= 6;
		startup->random += g_temp_buffer[i];
		startup->random *= shift_out + 1;
		startup->random ^= initial_value;
	}
	startup->bytes_fed += max_bytes_count;
	startup->blocked_until.tv_sec = now.tv_sec + startup->random % 2 + 1;
	startup->blocked_until.tv_nsec = now.tv_nsec + startup->random % 250000000;
	if (startup->blocked_until.tv_nsec > 1000000000) {
		++startup->blocked_until.tv_sec;
		startup->blocked_until.tv_nsec -= 1000000000;
	}
	timeout = startup->blocked_until;
	spin_unlock(&g_lock);

	LOG("Startup \"%.100s\" blocked until %li.%09li.", name, timeout.tv_sec, timeout.tv_nsec);
	
	return max_bytes_count;
}

static const int g_numerators[] = { -4, -1, 1, 2 };
static ssize_t csgames_read(struct file* file, char __user* buf, size_t count, loff_t* ppos) {
	struct timespec now, timeout;
	struct startup_net_worth worth = {};
	struct startup_block* startup;
	char name[CSGAMES_MAX_STARTUP_NAME_SIZE];
	size_t move_bytes;
	unsigned random;
	unsigned magic;
	int i;
	
	if (csgames_run_angel()) {
		return -EINTR;
	}
	
	if (count != sizeof(worth)) {
		LOG("Can only read exactly %lu bytes!", sizeof(worth));
		return -EINVAL;
	}
	
	getnstimeofday(&now);
	
	spin_lock(&g_lock);
	startup = get_current_startup();
	if (startup == NULL) {
		spin_unlock(&g_lock);
		
		LOG("No startup selected!");
		return -EINVAL;
	}
	
	if (startup == &g_startups[g_angels_pick]) {
		spin_unlock(&g_lock);
		
		LOG("Trying to feed data to startup selected by angel investor!");
		return -EPERM;
	}
	
	if (is_before(&now, &startup->blocked_until)) {
		strncpy(name, startup->s.name, sizeof(name));
		++startup->blocked_until.tv_sec;
		timeout = startup->blocked_until;
		spin_unlock(&g_lock);
		
		LOG("Startup \"%.100s\" blocked until %li.%09li!", name, timeout.tv_sec, timeout.tv_nsec);
		return -EAGAIN;
	}
	
	worth.startup = startup->s;
	random = startup->random;
	move_bytes = (&g_startups[CSGAMES_MAX_STARTUPS] - startup - 1) * sizeof(*startup);
	memmove(startup, startup + 1, move_bytes);
	g_current_startup = CSGAMES_MAX_STARTUPS;
	g_startup_count--;
	spin_unlock(&g_lock);
	
	LOG("Ending processing for startup \"%.100s\".", worth.startup.name);
	
	worth.worth_by_year[0] = worth.startup.capital_in_dollars;
	magic = random;
	for (i = 1; i < CSGAMES_STARTUP_PREDICTED_YEAR_COUNT; ++i) {
		worth.worth_by_year[i] = worth.worth_by_year[i-1] * 6 / 5;
		worth.worth_by_year[i] += worth.worth_by_year[i] * g_numerators[magic & 0b11] / 5;
		magic >>= 2;
	}
	
	if (copy_to_user(buf, &worth, sizeof(worth)) != 0) {
		return -EFAULT;
	}

	i = __COUNTER__;
#define WBY (worth.worth_by_year[__COUNTER__ - 1 - i])
	LOG("*** {\"name\":\"%.100s\",\"worth\":[%li,%li,%li,%li,%li,%li,%li,%li,%li,%li,%li,%li,%li,%li,%li,%li]}",
		worth.startup.name, WBY, WBY, WBY, WBY, WBY, WBY, WBY, WBY, WBY, WBY, WBY, WBY, WBY, WBY, WBY, WBY);
#undef WBY
	
	return sizeof(worth);
}

#pragma mark - Running
static struct file_operations g_startup_simulator_fops = {
	.open = csgames_open,
	.release = csgames_close,
	.read = csgames_read,
	.write = csgames_write,
	.unlocked_ioctl = csgames_ioctl,
};

static struct miscdevice g_startup_simulator = {
	.name = CSGAMES_DEVICE_NAME,
	.fops = &g_startup_simulator_fops,
};

static int csgames_run_angel() {
	char startup_name[100];
	int startup_index;
	int picked = 0;
	int task_signalled = 0;
	
	spin_lock(&g_lock);
	if (!g_angel_enabled || g_angels_pick != CSGAMES_MAX_STARTUPS) {
		spin_unlock(&g_lock);
		return 0;
	}
	
	g_angel_state += 3;
	g_angel_state *= 48271;
	g_angel_state &= 0x7fffffff;
	startup_index = g_angel_state % (CSGAMES_MAX_STARTUPS * 80);
	if (startup_index < g_startup_count) {
		strncpy(startup_name, g_startups[startup_index].s.name, sizeof(startup_name));
		g_angels_pick = startup_index;
		picked = 1;
		if (g_angel_target != NULL) {
			task_signalled = g_angel_target->pid;
			send_sig(SIGNGEL, g_angel_target, 0);
		}
	}
	spin_unlock(&g_lock);
		
	if (task_signalled) {
		LOG("*** Signalled process %u of angel investment in startup \"%.100s\".", task_signalled, startup_name);
	}
	return picked;
}

static int __init csgames_init() {
	int result;
	
	g_startup_simulator.minor = MISC_DYNAMIC_MINOR;
	result = misc_register(&g_startup_simulator);
	if (result != 0) {
		LOG("Failed to register startup simulator! (error=%i)", result);
		return result;
	}
	
	return 0;
}

static void __exit csgames_exit() {
	misc_deregister(&g_startup_simulator);
	
	LOG("Startup simulator unloaded.");
}

module_init(csgames_init);
module_exit(csgames_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Félix Cloutier");
MODULE_DESCRIPTION("An awesome startup simulator");
