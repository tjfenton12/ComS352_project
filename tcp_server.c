#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define ENCRYPT 6
#define DECRYPT 6

#define LOCK 0
#define UNLOCK 1

/* last 5 digits of uid */
const unsigned int port = 70196;
const int buffer_size = 100;

typedef struct shared_memory {
	int pids[20];
	int gate;
	int num_pids;
} shared_memory;

/* Headers */
char **tokenize(char *message);
int find_length(char *str);
char * t_encrypt(char str[], int length);
char * t_decrypt(char str[], int length);
void t_wait(shared_memory *mutex);
void t_signal(shared_memory *mutex);
void add_pid(int pid, shared_memory *mem);
void remove_pid(int pid, shared_memory *mem);
void remove_character(char *str, char to_remove);

/* SERVER */
int main() {
	int global_persist = 1;
	char server_message[] = "You have reached the server!";
	key_t shmkey;
	int shmID;
	shared_memory *mem;
	int og_pid;

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

	/* set up shared memory */
	shmkey = ftok(".", 'x');
	shmID = shmget(shmkey, sizeof(shared_memory), IPC_CREAT | 0666);
	mem = shmat(shmID, NULL, 0);

	mem->gate = LOCK;
	mem->num_pids = 0;
	og_pid = getpid();
	add_pid(og_pid, mem);
	mem->gate = UNLOCK;

	shmdt((void *) mem);

	while(global_persist) {
		/* accept a client connection */
		int local_persist = 1;
		int client_socket;
		client_socket = accept(server_socket, NULL, NULL);
		
		int pid;
		pid = fork();
		if(pid == 0) {
			mem = (shared_memory *) shmat(shmID, NULL, 0);
			int this_pid;

			this_pid = (int) getpid();
			printf("%d", this_pid);
			/* Entering CS */
			t_wait(mem);
			add_pid(this_pid, mem);
			/* Leaving CS */
			t_signal(mem);

			/* recieve message */
			char client_message[256];
			recv(client_socket, &client_message, sizeof(client_message), 0);
			printf("recieved:<%s> from the client. \n", client_message);
			int length = find_length(client_message);
			char * decrypted_client_message = (char *) malloc(length);
			decrypted_client_message = t_decrypt(client_message, length);
			
			if(strcmp(decrypted_client_message, "jobs") == 0) {
				int i;
				/* entering critical section */
				t_wait(mem);
				char *job = (char *) malloc(256);
				for(i = 0; i < 20; i++) {
					snprintf(job, 256, "%d, ", mem->pids[i]);
					send(client_socket, job, sizeof(job), 0);
				}
				t_signal(mem);
			} 
			/* decrypted client message is not a custom command */
			else {
				/* tokenize the command from the client */
				char **tokens;
			       	tokens = tokenize(decrypted_client_message);
			
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
			/* Entering CS */
			t_wait(mem);
			remove_pid(this_pid, mem);
			/* Leaving CS */
			t_signal(mem);

			shmdt((void *) mem);
			/* close client socket */
			close(client_socket);
		}
		/* parent process */
	       	else {
			signal(SIGCHLD, SIG_IGN);
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
	return decrypted;
}

/**
 * Waits for and acquires mutex lock.
 *
 * shared_memory *mutex: pointer to the shared memory holding the mutex.
 */
void t_wait(shared_memory *mutex) {
	while(mutex->gate <= LOCK){}
	mutex->gate = LOCK;
}

/**
 * Gives up mutex lock.
 *
 * shared_memory *mutex: pointer to the shared memory holding the mutex.
 */
void t_signal(shared_memory *mutex) {
	mutex->gate = UNLOCK;
}

/**
 * Adds given pid to the list of pids in the shared memory.
 *
 * int pid: The pid to be added to the list.
 * shared_memory *mem: The shared memory.
 */
void add_pid(int pid, shared_memory *mem) {
	if(mem->num_pids == 20) {
		return;
	}
	mem->pids[mem->num_pids] = pid;
	mem->num_pids++;
}

/**
 * Removes a give pid from the list of pids in the shared memory. ASSUMES THAT PID EXISTS.
 *
 * int pid: The pid to be removed from the list.
 * shared_memory *mem: The shared memory.
 */
void remove_pid(int pid, shared_memory *mem) {
	int i, j, max;

	max = mem->num_pids;
	for(i = 0; i < max; i++) {
		if(mem->pids[i] == pid) {
			mem->pids[i] = 0;
			break;
		}
	}
	mem->num_pids--;
	for(j = i; j < max - 1; j++) {
		mem->pids[j] = mem->pids[j + 1];
	}
	mem->pids[max - 1] = 0;
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

