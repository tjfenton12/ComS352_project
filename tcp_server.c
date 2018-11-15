#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

/* last 5 digits of uid */
const unsigned int port = 70196;
const int buffer_size = 100;

char **tokenize(char *message);
int find_length(char *str);
char * t_encrypt(char str[], int length);
char * t_decrypt(char str[], int length);


/* SERVER */
int main() {
	int global_persist = 1;
	char server_message[] = "You have reached the server!";

	/* create the server socket */
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	/* define the server address */
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;

	/* bind the socket to our specified IP and port */
	bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
	
	/* listen for connections */
	listen(server_socket, 10);
	
	while(global_persist) {
		/* accept a client connection */
		int local_persist = 1;
		int client_socket;
		client_socket = accept(server_socket, NULL, NULL);
		
		int pid;
		pid = fork();
		if(pid == 0) {
			//while(local_persist) {
				/* recieve message */
				char client_message[256];
				recv(client_socket, &client_message, sizeof(client_message), 0);
				printf("recieved:<%s> from the client. \n", client_message);
				
				if(strcmp(client_message, "exit") == 0) {
					//local_persist = 0;
					printf("Exiting Process.");
			
				} else {
					/* tokenize the command from the client */
					char **tokens;
				       	tokens = tokenize(client_message);
				
					/* change the stderr and stdout to the client socket */
					dup2(client_socket, STDOUT_FILENO);
					dup2(client_socket, STDERR_FILENO);
				
					/* run the command from the client */	
					int error;
					error = execvp(tokens[0], tokens);
					if(error == -1){
						char execvp_msg_failure[] = "invalid command";
						send(client_socket, execvp_msg_failure, sizeof(execvp_msg_failure), 0);
					}
				}
			//}
			/* close client socket */
			close(client_socket);
		} /* parent process */
	       	else {
			/* close client socket */
			close(client_socket);
		}
	}

	/* close the server socket */
	close(server_socket);

	return 0;
}

/**
 * Creates and returns an array of strings which are the shell commands.
 *
 * char *message: The message sent to the server to be tokenized.
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

/**
 * Finds the length of a given string.
 * 
 * char *str: The string to find the length of.
 *
 * Returns: the length of the given string(including the terminating character).
 */
int find_length(char *str) {
	int i;
	for(i = -1; str[i] != '\0'; i++) {
		//Do Nothing
	}
	return i + 0;
}

/**
 * Encrypts a given string.
 *
 * char str[]: The string to be encrypted.
 * int length: The length of the string being encrypted.
 *
 * Returns: The encrypted string.
 */
char * t_encrypt(char str[], int length) {
	int i;
	char * encrypted = (char *) malloc(length * sizeof(char *));
	for(i = 0; i < length; i++) {
		encrypted[i] = str[i] + ENCRYPT;
	}
	return encrypted;
}

/**
 * Decrypts a given string.
 *
 * char str[]: The string to be decrypted.
 * int length: the length of the string being decrypted.
 *
 * Returns: The decrypted string.
 */
char * t_decrypt(char str[], int length) {
	int i;
	char * decrypted = (char *) malloc(length * sizeof(char *));
	for(i = 0; i < length; i++) {
		decrypted[i] = str[i] - DECRYPT;
	}
}
