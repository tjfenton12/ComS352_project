#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <unistd.h>

/* last 5 digits of uid */
const unsigned int PORT = 70196;

void remove_character(char *str, char to_remove);

/* CLIENT */
int main() {
	int persist = 1;
	
	while(persist) {	
		/* define a socket */
		int network_socket;
		network_socket = socket(AF_INET, SOCK_STREAM, 0);
	
		/* specify address for the socket to connect to */
		struct sockaddr_in server_address;
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(PORT);
		server_address.sin_addr.s_addr = INADDR_ANY;
	
		printf("Connecting to server.\n");
		/* call connect function */
		int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	
		/* check for error with the connection */
		if (connection_status == -1) {
			printf("There was an error making a connection to the remote socket \n\n");
			persist = 0;
		} else {
			printf("Connected to server.\n");
		}
		
		/* assumes that the message will be smaller than 256 bytes */
		char client_message[256];
		printf(">");
		fgets(client_message, 256, stdin);
		remove_character(client_message, '\n');
	
		/* when the user doesn't input exit */
		if (strcmp(client_message, "exit") != 0) {
			/* send data to the server */
			ssize_t size = send(network_socket, client_message, sizeof(client_message), 0);

			printf("The server sent the following data: \n");	
			/* recieve data from the server */
			char server_response[256];
			while(read(network_socket, &server_response, sizeof(server_response)) != 0) {
					printf("%s", server_response);
			}
			fflush(stdout);
			printf("\n");

		} else if (strcmp(client_message, "") == 0) {
			printf("Nothing to send. please try again.\n");
		} else {
			send(network_socket, client_message, sizeof(client_message), 0);
			persist = 0;
		}	

		/* close the socket */
		close(network_socket);

	}

	return 0;
}

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
