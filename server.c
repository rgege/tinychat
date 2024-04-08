#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <assert.h>

#include "netlib.h"
#include "memlib.h"

#define MAX_CLIENTS 1000
#define SERVER_PORT 3434

/* client representation */
struct client {
	int fd;
	char *nick;
};

/* encapsulates global state of the chat */
struct chatState {
	int serversock;
	int numclients;
	int maxclient;
	struct client *clients[MAX_CLIENTS];
};

struct chatState *tinyChat;

/* ====================== core implementation =============================
 * accept new connections, 
 * read what clients write, 
 * fan-out the message to every clien (except sender).
 * ===================================================================== */

/* new client creation and allocation */ 
struct client *createClient(int fd) {
	char nickname[32];
	/* default user nickname created */
	int nicklen = snprintf(nickname, sizeof(nickname), "user:%d", fd);
	struct client *c = tinyMalloc(sizeof(*c));	
	c->fd = fd;
	c->nick = tinyMalloc(sizeof(nicklen+1));
	memcpy(c->nick, nickname, nicklen+1);
	assert(tinyChat->clients[c->fd] == NULL); /* must be available */
	/* 'registering' new client to the server */
	tinyChat->clients[c->fd] = c;
	/* updating maxclient count if needed */
	if (c->fd > tinyChat->maxclient) tinyChat->maxclient = c->fd;
	tinyChat->numclients++;
	return c;
}

/* server initialisation and allocation */
void init(void) {
	tinyChat = tinyMalloc(sizeof(*tinyChat));
	memset(tinyChat, 0, sizeof(*tinyChat));
	tinyChat->numclients = 0;
	tinyChat->maxclient = -1;
	tinyChat->serversock = createTCPServer(SERVER_PORT);
	if (tinyChat->serversock == -1) {
		perror("initialisation failed");
		exit(EXIT_FAILURE);
	}
}

/* ====================== main() chat logic = =============================
 * accept new clients,
 * check if any client sent new msg,
 * send that msg to all other cliets,
 * ===================================================================== */

int main(void) {
	fd_set readfds;
	struct timeval tv;
	FD_ZERO(&readfds); 

	init();

	while (1) {
		/* track server socket for new activity */
		FD_SET(tinyChat->serversock, &readfds);

		for (int i = 0; i <= tinyChat->maxclient; ++i) 
			if (tinyChat->clients[i]) FD_SET(i, &readfds);

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		int maxfd = tinyChat->maxclient;
		if (maxfd < tinyChat->serversock) 
			maxfd = tinyChat->serversock;
		int retval = select(maxfd+1, &readfds, NULL, NULL, &tv);
		if (retval == -1) {
			perror("select failed");
			exit(EXIT_FAILURE);
		} else if(retval) {
			/* are there any pending connections? */
			if (FD_ISSET(tinyChat->serversock, &readfds)) {
				int fd = acceptClient(tinyChat->serversock);
				/* new client connected */
				struct client *c = createClient(fd);
				char *welcome_msg = 
					"Welcome to Tiny Chat!";
				write(c->fd, welcome_msg, strlen(welcome_msg));
				printf("Connected client: fd=%d\n", fd);
			}
		}

		char readbuf[256];
		for (int i = 0; i <= tinyChat->maxclient; ++i) {
			if (tinyChat->clients[i] == NULL) continue;
			if (FD_ISSET(i, &readfds)) {
				int nread = read(i, readbuf, sizeof(readbuf)-1);
				if (nread <= 0) {
				     	printf("Disconnected client: %s, fd=%d\n", 
						tinyChat->clients[i]->nick, i);
					tinyChat->clients[i] = NULL;
				} else {
					puts("user wants to send a msg");
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
