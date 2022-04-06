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

// define the port
#define PORT 8080 
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_OUTPUT 1024 // max length of output string
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument

int main(){

	// _____________________________socket code pt 1(socket code inspired by the code from lab 7)
	//1. create a socket
	int client_socket;
	client_socket = socket(AF_INET , SOCK_STREAM, 0); //socket( domain/family of the socket, type of socket,  protocol for connection)

	// error checking
	if(client_socket == -1)
	{
		perror("opening stream socket");
		exit(1);
	}

	
	// _____________________________connect to server socket/shell code
	// create the struct and define it's properties like family, port and so on
	struct sockaddr_in shell_address;
	//define family 
	shell_address.sin_family = AF_INET; 
	//specify port number
	shell_address.sin_port = htons(PORT); 
	//we use special IP address INADDR_ANY to bind our socket to an IP address of the machin
	shell_address.sin_addr.s_addr = INADDR_ANY;


	// int connection_status = connect(socket name, pointer to the shell socket address struct, size of address)
	int connection_status = connect(client_socket, (struct sockaddr *) &shell_address, sizeof(shell_address));

	// error checking 
	if (connection_status == -1) 
	{
		perror("connection to remote socket failed");
		exit(1);
	}

	// _____________________________end of socket code pt 1


	char buffer[MAX_BUF];
	char output[MAX_OUTPUT];

	printf("Client : \n");
	printf("---Welcome to the shell---\n");
	while (1) { // repeat

		printf("$ ");

		// _____________________________recieve user input and send to shell through socket
		// fill the buffer with NULL bytes 
		bzero(buffer, sizeof(buffer)); // place nbyte null bytes in the string s
		get_user_input(buffer, sizeof(buffer)); // get user input and store it in the buffer
		send(client_socket, buffer, sizeof(buffer), 0);
		// _____________________________recieve user input and send to shell through socket


		// _____________________________recieve command output from shell socket and print results
		bzero(output, sizeof(output));

		recv(client_socket, &output, sizeof(output), 0);
		if (strcmp(output, "EXIT") == 0) {
			break;
		}
		else
			printf("Result: %s\n", output);
	}
	// _____________________________recieve command output from shell socket and print results

	// close socket after we are done
	close(client_socket);
	return 0;

}