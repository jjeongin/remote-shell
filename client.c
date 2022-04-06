#define _GNU_SOURCE
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

#define PORT 8080 //or 8080 or any other unused port value
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_OUTPUT 1024 // max length of output string
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument

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
	if (connection_status == -1) {
		printf("There was an error making a connection to the remote socket \n\n");
		exit(EXIT_FAILURE);
	}

	char buffer[MAX_BUF];
	char output[MAX_OUTPUT];

	// # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #
	// # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #
	// # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #

	printf("Client : \n");
    printf("---Welcome to the shell---\n");
    while (1) { // repeat
		// shell interface
        // ask for the server's cwd ?
		// char * cwd = getcwd(NULL, 0); // print current working directory
		// printf("%s $ ", cwd);
		printf("$ ");
		bzero(buffer, sizeof(buffer)); // place nbyte null bytes in the string s
		get_user_input(buffer, sizeof(buffer)); // get user input and store it in the buffer
        send(network_socket, buffer, sizeof(buffer), 0);

        bzero(output, sizeof(output));

        recv(network_socket, &output, sizeof(output), 0);
        if (strcmp(output, "EXIT") == 0) {
        	break;
        }
        else
        	printf("%s\n", output);
        	// printf("Result: %s\n", output);
    }

	// close socket after we are done
	close(network_socket);

    // free the allocated memories
	// free(buffer);
	// free(output);
	return 0;

}