#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <unistd.h>

/* last 5 digits of uid */
const unsigned int port = 70196;

/* SERVER */
int main() {
	int persist = 1;
	char server_message[256] = "You have reached the server!";

	/* create the server socket */
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0); //Parameters: internet socket, tcp socket, flag
	
	/* define the server address */
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;

	/* bind the socket to our specified IP and port */
	bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
	
	/* listen for connections */
	listen(server_socket, 10);

	char client_message[256];
	int client_socket;
	client_socket = accept(server_socket, NULL, NULL);
		
	/* recieve message */
	recv(client_socket, &client_message, sizeof(client_message), 0);
	printf("recieved:\"%s\" from the server \n", client_message);

	/* send message */
	send(client_socket, server_message, sizeof(server_message), 0);

	/* close the socket */
	close(server_socket);

	return 0;
}
