#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <unistd.h>

/* last 5 digits of uid */
const unsigned int port = 70196;
const int buffer_size = 100;

char **tokenize(char *message);

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

	/* tokenize the command from the client */
	char **tokens = tokenize(client_message);
	printf("%s", tokens[0]);
	/* run the command from the client */	

	/* send message */
	send(client_socket, server_message, sizeof(server_message), 0);

	/* close the client socket */
	close(client_socket);

	/* close the server socket */
	close(server_socket);

	return 0;
}

/**
 * Creates and returns an array of strings which are the shell commands.
 *
 * char *message: the message sent to the server to be tokenized.
 * char *delim: the delimeter to tokenize the message on.
 */
char **tokenize(char *message) {
	int buffer_size = 100;
	char *new_message = message;
	//char *context_ptr;
	char *command = strtok(new_message, " ");
	char **args = (char**) malloc(buffer_size*sizeof(char *));

	int i;
	i = 0;
	while(command != NULL) {
		printf("%s", command);
		args[i] = command;
		command = strtok(NULL, " ");
		i++;
		if(i > buffer_size) {
			buffer_size += buffer_size;
			args = (char **) realloc(args, buffer_size * sizeof(char *));
		}
	}
	return args;
}
