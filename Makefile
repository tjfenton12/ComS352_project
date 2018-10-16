CC=gcc
CFLAGS=-ggdb

all: tcp_client.c tcp_server.c
	$(CC) $(CFLAGS) -o tcp_client tcp_client.c
	$(CC) $(CLFAGS) -o tcp_server tcp_server.c

clean:
	rm tcp_client tcp_server
