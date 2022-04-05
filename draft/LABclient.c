#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "command.h"

#define PORT 9002 //or 8080 or any other unused port value
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument
#define COMMANDS 13 // number of possible commands
#define _GNU_SOURCE


int main(){

	//create a socket
	int network_socket;
	network_socket = socket(AF_INET , SOCK_STREAM, 0);

	//check for fail error
	if (network_socket == -1) {
        printf("socket creation failed..\n");
        exit(EXIT_FAILURE);
    }
    

	//connect to another socket on the other side
	
	//specify an address for the socket we want to connect to
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;


	//connect
	int connection_status = 
	connect(network_socket, 
			(struct sockaddr *) &server_address,
			sizeof(server_address));

	//check for errors with the connection
	if(connection_status == -1){
		printf("There was an error making a connection to the remote socket \n\n");
		exit(EXIT_FAILURE);
	}
	

	//now that we have the connect, we either send or receive data



	// next steps:
		// use the getInput function
		// loop over the whole thing
	// # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #

	// allocate memory for buffer and argument list
	char * buffer; // allocate memory for buffer
	size_t bufsize = MAX_BUF;
	buffer = (char *) malloc(bufsize * sizeof(char));
	if (buffer == NULL) {
		perror("Failed to allocate buffer.\n");
		exit(1);
	}

	char ** args = (char **) malloc(MAX_ARGS * sizeof(char*)); // allocate memory for argument list (https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc)
	for (int i = 0; i < MAX_ARGS; i++) {
		args[i] = malloc((MAX_ARG_LEN+1) * sizeof(char));

 		if (args == NULL) {
			perror("Failed to allocate argument list.\n");
			exit(1);
		}
	}

	getline(&buffer, &bufsize, stdin);
	buffer[strcspn(buffer, "\n")] = 0; // remove new line character at the end (https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input)

    printf("buffer: %s\n", buffer);
    send(network_socket , buffer , bufsize,0);

	char output[1024];
	recv(network_socket , &output , sizeof(output),0);
	printf("%s\n"output);



	//close socket after we are done
	close(network_socket);



	return 0;
}