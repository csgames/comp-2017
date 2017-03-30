#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <time.h>

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
	long worth_by_year[CSGAMES_STARTUP_PREDICTED_YEAR_COUNT];
};

enum csgames_ioctl_request_code {
	CSGAMES_NEW_STARTUP = 0x100,		// (const struct startup*)
	CSGAMES_SET_STARTUP,		// (const char*)
	CSGAMES_GET_PROCESSING_END,	// (struct timeval*)
	CSGAMES_GET_ANGELS_PICK,	// (char [CSGAMES_MAX_STARTUP_NAME_SIZE])
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
int main() {
  // Socket stuff
	const struct sockaddr_in bind_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(4321),
		.sin_addr = { htonl(INADDR_LOOPBACK) },
	};

	int acceptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (acceptor < 0) {
		perror("socket");
		return 1;
	}

	int result;
	int yes = 1;
	result = setsockopt(acceptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if (result < 0) {
		perror("setsockopt");
		return 1;
	}

	result = bind(acceptor, (const struct sockaddr*)&bind_addr, sizeof(bind_addr));
	if (result < 0) {
		perror("bind");
		return 1;
	}

	result = listen(acceptor, CSGAMES_MAX_STARTUPS);
	if (result < 0) {
		perror("listen");
		return 1;
	}

	while (1) {
		struct sockaddr_in client_address;
		socklen_t client_address_length = sizeof(client_address);
		int client = accept(acceptor, (struct sockaddr*)&client_address, &client_address_length);
		if (client < 0) {
			perror("accept");
			continue;
		}

		// MY WORK HERE IS DONE!
	}
}
