#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

int createTCPServer(int port) {
	
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags 	  = AI_PASSIVE;

	int stat;
	char portnum[6];
	snprintf(portnum, sizeof(portnum),"%d", port);
	if ((stat = getaddrinfo(NULL, portnum, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo failed with error: %s\n",
				 gai_strerror(stat));
		return -1;
	}

	int s;
	if ((s = socket(res->ai_family, res->ai_socktype, 
		res->ai_protocol)) == -1) {
		perror("socket creation failed");
		return -1;
	}

	int yes = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
		== -1) {
		perror("setsockopt failed");
		return -1;
	}

	if (bind(s, res->ai_addr, res->ai_addrlen) == -1) {
		close(s);
		perror("bind failed");
		return -1;
	}

	if (listen(s, SOMAXCONN) == -1) {
		close(s);
		perror("listen failed");
		return -1;
	}

	return s;
}

int acceptClient(int server_socket) {
	int s;
	while (1) {
	       struct sockaddr_storage addr;
	       socklen_t slen = sizeof(addr);
	       s = accept(server_socket, (struct sockaddr*)&addr, &slen);
	       if (s == -1) {
			if (errno == EINTR)
				continue;
			else {
				perror("accept failed");
				return -1;
			}
		}
		break;
	}
	return s;
}

int socketNonBlockNoDelay(int fd) {
	int flags, yes = 1;

	if ((flags = fcntl(fd, F_GETFL)) == -1) return -1;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) return -1;

	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int));
	return 0;
}
