#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define ENCRYPT 6
#define DECRYPT 6

/* last 5 digits of uid */
const unsigned int PORT = 70196;

/**
 * Structure that holds:
 * 	an integer representation of the tail of the queue
 * 	the queue
 */
typedef struct history_queue {
	int tail;
	char *queue[10];
} history_queue;

/* Headers */
void remove_character(char *str, char to_remove);
int find_length(char *str);
char * t_encrypt(char str[], int length);
char * t_decrypt(char str[], int length);
void add_command(char *addition, history_queue *history);
char * peek_command(int command, history_queue *history);
void print_history(history_queue *history);
int find_semicolons(char *str, int length);
char **tokenize(char *message);

/* CLIENT */
int main() {
	history_queue *history;
	int persist;
	int take_commands; /* 1 for yes, 0 for no */
	int num_commands;
	int command_place;
	char **tokens;

	take_commands = 1;
	num_commands = 0;
	command_place = 0;
	history = (history_queue *) malloc(sizeof(history_queue));
	history->tail = 0;
	persist = 1;

	while(persist) {
		char *client_message;
		char *encrypted_client_message;
		int length;

		if(take_commands == 1) {
			/* assumes that the message will be smaller than 256 bytes */
			client_message = (char *) malloc(256);
			printf(">");
			fgets(client_message, 256, stdin);
			remove_character(client_message, '\n');
			length = find_length(client_message);
			num_commands = find_semicolons(client_message, length);
			if(num_commands > 0) {
				take_commands = 0;
				tokens = tokenize(client_message);
				command_place = 0;
			}
			encrypted_client_message = (char *) malloc(length);
			encrypted_client_message = t_encrypt(client_message, length);
		}
		/* define a socket */
		int network_socket;
		network_socket = socket(AF_INET, SOCK_STREAM, 0);
	
		/* specify address for the socket to connect to */
		struct sockaddr_in server_address;
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(PORT);
		server_address.sin_addr.s_addr = INADDR_ANY;
	
		printf("Connecting to server...\n");
		/* call connect function */
		int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	
		/* check for error with the connection */
		if (connection_status == -1) {
			printf("There was an error making a connection to the remote socket \n\n");
			persist = 0;
		} else {
			printf("Connected to server!\n");
		}
		
		/* if the client message contains multiple commands */
		if (take_commands == 0) {
			if(strcmp(tokens[command_place], "history") == 0) {
				print_history(history);
				add_command(tokens[command_place], history);
			} else {
				char *encrypted_command;
				int command_length;

				command_length = find_length(tokens[command_place]);
				encrypted_command = (char *) malloc(command_length);
				encrypted_command = t_encrypt(tokens[command_place], command_length);
				add_command(tokens[command_place], history);
				/* send data to the server */
				ssize_t size = send(network_socket, encrypted_command, sizeof(encrypted_command), 0);

				printf("The server sent the following data: \n");
				/* recieve the data from the server */
				char server_response[256];
				while(read(network_socket, &server_response, sizeof(server_response)) != 0) {
					printf("%s", server_response);
				}
				fflush(stdout);
				printf("\n");
			}
			command_place++;
			/* if we've performed all of the commands, reset */
			if(command_place == num_commands + 1) {
				take_commands = 1;
				command_place = 0;
				num_commands = 0;
			}
		}
		/* if the client message is "history" */
		else if (strcmp(client_message, "history") == 0) {
			print_history(history);
			add_command(client_message, history);
		} 
		/* if the client message starts with '!' and is length 2 */
		else if ((client_message[0] == '!') && ((find_length(client_message) - 1) == 2)) {
			char char_test = client_message[1];
			int int_test = client_message[1] - '0';
			/* if second character is '!' */
			if (char_test == '!') {
				if(history->tail == 0) {
					printf("No commands in history");
				}
				/* if the most recent command is "history" */
				else if(strcmp(peek_command(1, history), "history") == 0) {
					add_command(peek_command(1, history), history);
					print_history(history);
				} else {
					add_command(peek_command(1, history), history);
					int command_length = find_length(peek_command(1, history));
					char * command = (char *) malloc(length);
					command = peek_command(1, history);
					char * encrypted_command = (char *) malloc(command_length);
					encrypted_command = t_encrypt(command, command_length);
					/* send data to the server */
					ssize_t size = send(network_socket, encrypted_command, sizeof(encrypted_command), 0);

					printf("The server sent the following data: \n");	
					/* recieve data from the server */
					char server_response[256];
					while(read(network_socket, &server_response, sizeof(server_response)) != 0) {
						printf("%s", server_response);
					}
					fflush(stdout);
					printf("\n");
				}
			}
			/* if second character is a valid number between 1 and 10 */
			else if ((int_test > 0) && (int_test < 11)) {
				if(int_test > history->tail) {
					printf("No such command in history.");
				}
				/* if the most recent command is "history" */
				else if(strcmp(peek_command(int_test, history), "history") == 0) {
					add_command(peek_command(int_test, history), history);
					print_history(history);
				} else {
					printf("%d\n", int_test); 
					add_command(peek_command(int_test, history), history);
					int command_length = find_length(peek_command(int_test, history));
					char * command = (char *) malloc(length);
					command = peek_command(int_test, history);
					char * encrypted_command = (char *) malloc(command_length);
					encrypted_command = t_encrypt(command, command_length);

					/* send data to the server */
					ssize_t size = send(network_socket, encrypted_command, sizeof(encrypted_command), 0);

					printf("The server sent the following data: \n");	
					/* recieve data from the server */
					char server_response[256];
					while(read(network_socket, &server_response, sizeof(server_response)) != 0) {
						printf("%s", server_response);
						sleep(1);
					}
					fflush(stdout);
					printf("\n");
				}
			}
		} 
		/* if client message is exit */
		else if (strcmp(client_message, "quit") == 0) {
			persist = 0;
		}
		/* if client message is empty */
		else if (strcmp(client_message, "") == 0) {
			printf("Nothing to send. Please try again.\n");
		}
		/* if client message is not a custom command */
		else {
			add_command(client_message, history);
			/* send data to the server */
			ssize_t size = send(network_socket, encrypted_client_message, sizeof(encrypted_client_message), 0);

			printf("The server sent the following data: \n");	
			/* recieve data from the server */
			char server_response[256];
			while(read(network_socket, &server_response, sizeof(server_response)) != 0) {
				printf("%s", server_response);
			}
			fflush(stdout);
			printf("\n");
			//printf("%s\n", history->queue[0]);

		}	

		/* close the socket */
		close(network_socket);

	}

	free(history);
	return 0;
}

/**
 * Removes a specified character from a given string.
 *
 * char *str: The string to have a character removed from it.
 * char to_remove: The character to be removed from the string.
 */
void remove_character(char *str, char to_remove) {
	char *src, *dst;

	for (src = dst = str; *src != '\0'; src++) {
		*dst = *src;
		if (*dst != to_remove) {
			dst++;
		}
	}
	*dst = '\0';
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

	for(i = 0; str[i] != '\0'; i++) {
		//Do Nothing
	}
	return i + 1;
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

	char *encrypted = (char *) malloc(length * sizeof(char *));
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

	char *decrypted = (char *) malloc(length * sizeof(char *));
	for(i = 0; i < length; i++) {
		decrypted[i] = str[i] - DECRYPT;
	}
}

/**
 * Adds a given command to the provided history queue.
 *
 * char *addition: The command to be added.
 * history_queue *history: The queue struct to add the command to.
 */
void add_command(char *addition, history_queue *history) {
	int position = history->tail;
	int i;

	if(position == 0) {
		history->queue[0] = addition;
		history->tail++;
	} else {
		for(i = position; i > 0; i--) {
			history->queue[i] = history->queue[i - 1];
		}
		history->queue[0] = addition;
		if(position != 9) {
			history->tail++;
		}
	}
}

/**
 * Peeks at a command in the queue.
 *
 * int command: The command (provided between 1 to 10) to be peeked at.
 * history_queue: The queue struct to get the command from.
 *
 * returns: The string representing the peeked command.
 */
char * peek_command(int command, history_queue *history) {
	if(command >= 1 && command <= 10) {
		char *peeked_command = (char *) malloc(find_length(history->queue[command - 1]));
		peeked_command = history->queue[command - 1];
		return peeked_command;
	}
	return NULL;
}

/**
 * Helper function to print the command history.
 */
void print_history(history_queue *history) {
	int i;

	for(i = 0; i < history->tail; i++) {
		printf("%d: %s\n", i + 1, history->queue[i]); 
	}
}

/**
 * Looks to see if semicolons exist in the provided string.
 *
 * char *str: The string to scan for semicolons.
 * int length: The length of the provided string.
 *
 * returns: The number of semicolons which were found.
 */
int find_semicolons(char *str, int length) {
	int i, count;

	count = 0;
	for(i = 0; i < length; i++) {
		if(str[i] == ';') {
			count++;
		}
	}

	return count;
}

/**
 * Creates and returns an array of strings which are the shell commands.
 *
 * char *message: The message sent to the server to be tokenized.
 *
 * returns: The tokens representing different commands.
 */
char **tokenize(char *message) {
	int buffer_size = 100;
	char *new_message = message;
	//char *context_ptr;
	char *command = strtok(new_message, ";");
	char **args = (char**) malloc(buffer_size*sizeof(char *));

	int i;
	i = 0;
	while(command != NULL) {
		args[i] = command;
		command = strtok(NULL, ";");
		i++;
		if(i > buffer_size) {
			buffer_size += buffer_size;
			args = (char **) realloc(args, buffer_size * sizeof(char *));
		}
	}
	return args;
}
