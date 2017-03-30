#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace std;

#define LOG(fmt, ...) fprintf(stderr, "[SS:%u] " fmt "\n", getpid(), ## __VA_ARGS__)

// Driver stuff
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

enum csgames_ioctl_request_code {
	CSGAMES_NEW_STARTUP = 0x100,		// (const struct startup*)
	CSGAMES_SET_STARTUP = 0x101,		// (const char*)
	CSGAMES_GET_PROCESSING_END = 0x102,	// (timeval*)
	CSGAMES_GET_ANGELS_PICK = 0x103,	// (char [CSGAMES_MAX_STARTUP_NAME_SIZE])
};

// Socket stuff
const sockaddr_in bind_addr = {
	.sin_family = AF_INET,
	.sin_port = htons(4321),
	.sin_addr = { htonl(INADDR_LOOPBACK) },
};

// Write all numeric values in network byte order!
enum csgames_protocol_command_code {
	BEGIN_STARTUP,				// (uint32_t, char [])
	BUSINESS_PLAN_DATA,			// (const char [])
	
	ACKNOWLEDGED,				// (void)
	ANGEL_INVESTOR,				// (void)
	PREDICTION_RESULT,			// (long [CSGAMES_STARTUP_PREDICTED_YEAR_COUNT])
	ERROR,						// (const char [])
};

// Program
struct startup_entry {
	struct startup startup;
	timeval ready_at;
	pid_t pid;
};

startup_entry all_startups[CSGAMES_MAX_STARTUPS];
atomic_int startup_count;
int ss;
int global_client;
int lock;

struct command_header {
	unsigned short size;
	unsigned short command;
	
	unsigned short payload_size() const {
		return size - sizeof *this;
	}
};

struct begin_startup_command {
	command_header header;
	unsigned capital;
	char name[100];
};

struct business_plan_data_command {
	command_header header;
	uint8_t data[1];
};

struct [[gnu::packed]] prediction_result {
	command_header header;
	long values[CSGAMES_STARTUP_PREDICTED_YEAR_COUNT];
};

union command_buffer {
	command_header header;
	begin_startup_command begin_startup;
	business_plan_data_command business_plan_data;
	prediction_result prediction;
	uint8_t bytes[0xffff + sizeof(command_header)];
};

struct lock_file {
	int fd;
	lock_file(int fd) : fd(fd) { flock(fd, LOCK_EX); }
	~lock_file() { flock(fd, LOCK_UN); }
};

int read_exactly(int sock, void* buffer, ssize_t count) {
	uint8_t* cBuffer = reinterpret_cast<uint8_t*>(buffer);
	ssize_t readTotal = 0;
	while (readTotal != count) {
		ssize_t result = recv(sock, cBuffer + readTotal, count - readTotal, MSG_NOSIGNAL);
		if (result < 0 && errno != EINTR) {
			return errno;
		}
		else if (result == 0) {
			return EPIPE;
		}
		readTotal += result;
		if (readTotal != count)
		{
			LOG("Read %zi bytes out of %zi", readTotal, count);
		}
	}
	LOG("Read %zi bytes", readTotal);
	return 0;
}

int send_exactly(int sock, const void* buffer, ssize_t count) {
	const uint8_t* cBuffer = reinterpret_cast<const uint8_t*>(buffer);
	ssize_t sentTotal = 0;
	while (sentTotal != count) {
		ssize_t result = send(sock, cBuffer + sentTotal, count - sentTotal, MSG_NOSIGNAL);
		if (result < 0 && errno != EINTR) {
			return errno;
		}
		else if (result == 0) {
			return EPIPE;
		}
		sentTotal += result;
		if (sentTotal != count)
		{
			LOG("Wrote %zi bytes out of %zi", sentTotal, count);
		}
	}
	LOG("Sent %zi bytes", sentTotal);
	return 0;
}

bool before(const timeval& a, const timeval& b) {
	return a.tv_sec < b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_usec < b.tv_usec);
}

int send_error(int client, const char* fmt, ...) {
	char* message;
	
	va_list ap;
	va_start(ap, fmt);
	size_t len = vasprintf(&message, fmt, ap);
	va_end(ap);
	
	command_header header = {
		.size = htons(sizeof(command_header) + len),
		.command = htons(ERROR),
	};
	
	int result = send_exactly(client, &header, sizeof(header));
	if (result == 0) {
		result = send_exactly(client, message, len);
		LOG("Error to client %i: %s", client, message);
	}
	free(message);
	return result;
}

void serve_client(int client, startup_entry* entry) {
	global_client = client;
	vector<uint8_t> bytes;
	int result;
	
	const command_header ack = { htons(4), htons(ACKNOWLEDGED) };
	// Send ACK back for begin_startup
	if (send_exactly(client, &ack, sizeof ack) != 0) {
		return;
	}
	
	// read input
	while (true) {
		LOG("Ready for command");
		command_buffer buffer;
		result = read_exactly(client, buffer.bytes, sizeof buffer.header);
		if (result != 0) {
			bytes.clear();
			break;
		}
		
		buffer.header.command = ntohs(buffer.header.command);
		buffer.header.size = ntohs(buffer.header.size);
		if (buffer.header.size != 0) {
			result = read_exactly(client, buffer.business_plan_data.data, buffer.header.payload_size());
			if (result != 0) {
				LOG("Reading command %hu of size %hu failed! (error=%i)", buffer.header.command, buffer.header.size, result);
				bytes.clear();
				break;
			}
		}
		
		if (buffer.header.command == BUSINESS_PLAN_DATA) {
			LOG("Received %hu bytes of data plan", buffer.header.payload_size());
			if (buffer.header.payload_size() == 0) {
				LOG("Finished reading data plan.");
				break;
			}
			else {
				bytes.insert(bytes.end(), buffer.business_plan_data.data, buffer.bytes + buffer.header.payload_size());
				result = send_exactly(client, &ack, sizeof ack);
				if (result != 0) {
					LOG("Terminating receive loop because of receive error %i", result);
					bytes.clear();
					break;
				}
			}
		}
		else {
			result = send_error(client, "expected BUSINESS_PLAN_DATA command (%i), got %i instead!", BUSINESS_PLAN_DATA, buffer.header.command);
		}
	}
	
	// drain buffer
	timeval now = {};
	size_t index = 0;
	while (index != bytes.size()) {
		LOG("Acquiring lock");
		lock_file l(lock);
		result = ioctl(ss, CSGAMES_SET_STARTUP, entry->startup.name);
		if (result < 0) {
			if (errno == EINTR) {
				continue;
			} else {
				break;
			}
		}
		LOG("Selected startup %s", entry->startup.name);
		
		ssize_t writeCount = write(ss, &bytes[index], bytes.size() - index);
		if (writeCount < 0) {
			if (errno != EAGAIN) {
				LOG("Failed to write with error %i", errno);
				break;
			} else {
				LOG("*** THROTTLED!");
			}
		} else {
			index += writeCount;
			LOG("Wrote %zi bytes (%zu/%zu)", writeCount, index, bytes.size());
		}
		
		result = ioctl(ss, CSGAMES_GET_PROCESSING_END, &entry->ready_at);
		if (result < 0) {
			break;
		}
		gettimeofday(&now, nullptr);
		LOG("It is %li.%06li", now.tv_sec, now.tv_usec);
		if (before(now, entry->ready_at)) {
			LOG("Sleeping until %li.%06li", entry->ready_at.tv_sec, entry->ready_at.tv_usec);
			useconds_t sleep_time = entry->ready_at.tv_sec - now.tv_sec;
			sleep_time *= 1000000;
			sleep_time += entry->ready_at.tv_usec - now.tv_usec;
			usleep(sleep_time);
		}
	}
	
	LOG("Complete data uploaded");
	
	// read and send back result
	startup_net_worth output;
	{
		lock_file l(lock);
		result = ioctl(ss, CSGAMES_SET_STARTUP, entry->startup.name);
		if (result < 0) {
			LOG("Couldn't set startup! error %i", errno);
			return;
		}
		result = read(ss, &output, sizeof output);
		if (result < 0) {
			LOG("Couldn't read output! error %i", errno);
			return;
		}
	}
	
	prediction_result prediction;
	prediction.header.command = htons(PREDICTION_RESULT);
	prediction.header.size = htons(sizeof prediction);
	for (int i = 0; i < CSGAMES_STARTUP_PREDICTED_YEAR_COUNT; ++i) {
		unsigned char valueBuffer[sizeof(long)];
		memcpy(valueBuffer, &output.worth_by_year[i], sizeof output.worth_by_year[i]);
		reverse(begin(valueBuffer), end(valueBuffer));
		prediction.values[i] = *reinterpret_cast<long*>(valueBuffer);
	}
	send_exactly(client, &prediction, sizeof prediction);
}

void handle_client(int client) {
	LOG("New client %i", client);
	command_buffer buffer = {};
	int result;
	
	// THING TO TEST:
	// How do servers react when clients block here? (This implementation does
	// not do the right thing.)
	do
	{
		result = read_exactly(client, buffer.bytes, sizeof buffer.header);
		if (result != 0) {
			LOG("error %i reading command header!", result);
			return;
		}
		
		buffer.header.command = htons(buffer.header.command);
		buffer.header.size = htons(buffer.header.size);
		if (buffer.header.command != BEGIN_STARTUP) {
			send_error(client, "expected BEGIN_STARTUP (%u), got %hu instead!", BEGIN_STARTUP, buffer.header.command);
			continue;
		}
		if (buffer.header.size > sizeof(begin_startup_command)) {
			// THING TO TEST:
			// How do servers react to a buffer that is too large?
			send_error(client, "BEGIN_STARTUP command of size %hu is too large!", buffer.header.size);
			continue;
		}
		
		result = read_exactly(client, buffer.bytes + sizeof buffer.header, buffer.header.payload_size());
		if (result != 0) {
			LOG("error %i reading command body!", result);
			return;
		}
		buffer.begin_startup.capital = htonl(buffer.begin_startup.capital);
		buffer.bytes[buffer.header.size] = 0; // nullptr-terminate buffer
		break;
	} while (true);
	
	pid_t pid = -1;
	sigset_t set;
	sigprocmask(0, nullptr, &set);
	sigset_t oldMask = set;
	
	sigaddset(&set, SIGCHLD);
	sigaddset(&set, SIGNGEL);
	sigprocmask(SIG_SETMASK, &set, nullptr);
	
	int index = startup_count;
	memset(&all_startups[index], 0, sizeof all_startups[index]);
	all_startups[index].startup.capital_in_dollars = buffer.begin_startup.capital;

	size_t nameSize = buffer.begin_startup.header.size
		- sizeof(buffer.begin_startup.header)
		- sizeof(buffer.begin_startup.capital);
	strncpy(all_startups[index].startup.name, buffer.begin_startup.name, sizeof(all_startups[index].startup.name));
	
	LOG("Startup %s, capital %u", all_startups[index].startup.name, all_startups[index].startup.capital_in_dollars);
	
	result = ioctl(ss, CSGAMES_NEW_STARTUP, &all_startups[index].startup);
	if (result == 0)
	{
		pid = fork();
		if (pid == -1) {
			perror("fork");
		}
		else if (pid != 0) {
			all_startups[index].pid = pid;
			++startup_count;
		}
	}
	else
	{
		LOG("CSGAMES_NEW_STARTUP failed with %i", errno);
	}
	sigprocmask(SIG_SETMASK, &oldMask, nullptr);
	
	if (pid == 0) {
		LOG("forked!");
		serve_client(client, &all_startups[index]);
		
		// cleanup
		startup_net_worth net_worth;
		read(client, &net_worth, sizeof net_worth);
		exit(0);
	}
}

void sigchld(int, siginfo_t* info, void*) {
	switch (info->si_code)
	{
	case CLD_EXITED:
	case CLD_KILLED:
	case CLD_DUMPED:
		for (int i = 0; i < startup_count; ++i) {
			if (info->si_pid == all_startups[i].pid) {
				memmove(
					&all_startups[i],
					&all_startups[i+1],
					sizeof(all_startups[0]) * (CSGAMES_MAX_STARTUPS - i - 1));
				--startup_count;
				break;
			}
		}
		break;
	default: break;
	}
}

void signgel(int) {
	char startup_name[CSGAMES_MAX_STARTUP_NAME_SIZE];
	int err = ioctl(ss, CSGAMES_GET_ANGELS_PICK, &startup_name);
	if (err != 0) {
		static char message[] = "couldn't get angel's pick!\n";
		write(STDERR_FILENO, message, sizeof(message)-1);
		exit(1);
	}
	
	for (int i = 0; i < startup_count; ++i) {
		if (strncmp(startup_name, all_startups[i].startup.name, sizeof(startup_name)) == 0) {
			kill(all_startups[i].pid, SIGQUIT);
			return;
		}
	}
	static char message[] = "couldn't find startup!\n";
	write(STDERR_FILENO, message, sizeof(message)-1);
}

void sigquit(int) {
	command_header angel = { htons(sizeof angel), htons(ANGEL_INVESTOR) };
	write(global_client, &angel, sizeof angel);
	exit(SIGQUIT);
}

int main() {
	int result;

#pragma mark - Signal Handlers
	fprintf(stderr, "[SS] Setting up signal handlers... ");
	struct sigaction chldAction;
	result = sigaction(SIGCHLD, nullptr, &chldAction);
	if (result != 0) {
		perror("sigaction(SIGCHLD)");
		return 1;
	}
	chldAction.sa_sigaction = &sigchld;
	chldAction.sa_flags = SA_SIGINFO;
	sigaddset(&chldAction.sa_mask, SIGNGEL);
	sigaddset(&chldAction.sa_mask, SIGCHLD);
	result = sigaction(SIGCHLD, &chldAction, nullptr);
	if (result != 0) {
		perror("sigaction(SIGCHLD)");
		return 1;
	}
	
	struct sigaction ngelAction;
	result = sigaction(SIGNGEL, nullptr, &ngelAction);
	if (result != 0) {
		perror("sigaction(SIGNGEL)");
		return 1;
	}
	ngelAction.sa_handler = &signgel;
	sigaddset(&ngelAction.sa_mask, SIGNGEL);
	sigaddset(&ngelAction.sa_mask, SIGCHLD);
	result = sigaction(SIGNGEL, &ngelAction, nullptr);
	if (result != 0) {
		perror("sigaction(SIGNGEL)");
		return 1;
	}
	
	signal(SIGQUIT, &sigquit);
	fprintf(stderr, "done\n");

#pragma mark - Preparing lock
	char lockFileName[] = "csgames_startup_simulator.XXXXXX";
	lock = mkstemp(lockFileName);

#pragma mark - Opening device
	ss = open("/dev/startup_simulator", O_RDWR);
	if (ss < 0) {
		perror("open(startup_simulator)");
		return 1;
	}
	
#pragma mark - Opening sockets
	int acceptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (acceptor < 0) {
		perror("socket");
		return 1;
	}
	
	int yes = 1;
	result = setsockopt(acceptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if (result != 0) {
		perror("setsockopt");
		return 1;
	}
	
	result = bind(acceptor, (const struct sockaddr*)&bind_addr, sizeof(bind_addr));
	if (result != 0) {
		perror("bind");
		return 1;
	}
	
	result = listen(acceptor, 24);
	if (result != 0) {
		perror("listen");
		return 1;
	}
	
	while (1) {
		// THING TO TEST:
		// how do servers handle more than 8 clients?
		while (startup_count == 8) {
			pause();
		}
		
		LOG("Accepting new client.");
		struct sockaddr_in client_address;
		socklen_t client_address_length = sizeof(client_address);
		int client = accept(acceptor, (struct sockaddr*)&client_address, &client_address_length);
		if (client < 0) {
			perror("accept");
			continue;
		}
		
		// THINGS TO TEST:
		// How does a client handle being stuck waiting?
		handle_client(client);
		close(client);
	}
}
