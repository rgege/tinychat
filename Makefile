CFLAGS=-Wall -Werror -Wextra -Wpedantic;

all: server

server:	server.c netlib.c memlib.c
	$(CC) server.c netlib.c memlib.c -o server $(CFLAGS)

clean:
	rm -f server *.core
