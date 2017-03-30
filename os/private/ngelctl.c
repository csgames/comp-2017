#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, const char** argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "usage: %s [-s] startup-name\n", argv[0]);
		fprintf(stderr, "       %s -c\n", argv[0]);
		fprintf(stderr, "       %s -e\n", argv[0]);
		fprintf(stderr, "       %s -d\n", argv[0]);
		return 1;
	}
	
	int fd = open("/dev/startup_simulator", O_RDWR);
	if (fd < 0)
	{
		perror("open");
		return 1;
	}
	
	if (strcmp(argv[1], "-c") == 0) {
		char name[100];
		if (ioctl(fd, 0x103, name) < 0) {
			perror("clear angel's pick");
			return 1;
		}
	} else if (strcmp(argv[1], "-e") == 0) {
		if (ioctl(fd, 0x105, 1) < 0)
		{
			perror("enable angel");
			return 1;
		}
	} else if (strcmp(argv[1], "-d") == 0) {
		if (ioctl(fd, 0x105, 0) < 0)
		{
			perror("disable angel");
			return 1;
		}
	} else {
		int argN = 1;
		if (strcmp(argv[argN], "-s") == 0) {
			++argN;
		}

		char name[100];
		strncpy(name, argv[argN], sizeof(name));
		if (ioctl(fd, 0x104, name) < 0)
		{
			perror("set angel's pick");
			return 1;
		}
	}
	
	return 0;
}
