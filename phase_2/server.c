// to check:
	// pass string through with the commands
	//apply functions here
	//
// https://explainshell.com/explain?cmd=find%20.%20-type%20f%20-exec%20ls%20-s%20%7B%7D%20%5C;%20%7C%20sort%20-n%20-r%20%7C%20head.1posix%20-5
// https://www.unix.com/programming/88529-how-run-linux-command-redirect-its-output-socket-c.html
// socket
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
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include "command.h"

#define PORT 8080 //or 8080 or any other unused port value
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_OUTPUT 1024
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument
#define COMMANDS 12 // number of possible commands


int main()
{
	// _____________________________socket code pt 1(socket code inspired by the code from lab 7)
	//1. create a socket
	int shell_socket;
	shell_socket = socket(AF_INET , SOCK_STREAM, 0); //socket( domain/family of the socket, type of socket,  protocol for connection)

	// error checking
	if(shell_socket == -1)
	{
		perror("opening stream socket failed\n");
		exit(1);
	}
	// _____________________________end of socket code pt 1


	// _____________________________connect to server socket/shell code
	// create the struct and define it's properties like family, port and so on
	struct sockaddr_in shell_address;
	//define family 
	shell_address.sin_family = AF_INET; 
	//specify port number
	shell_address.sin_port = htons(PORT); 
	//we use special IP address INADDR_ANY to bind our socket to an IP address of the machin
	shell_address.sin_addr.s_addr = INADDR_ANY;
	// _____________________________connect to server socket/shell code


	// _____________________________bind the socket to the local address and error check
	if (bind(shell_socket, (struct sockaddr *) &shell_address, sizeof(shell_address)) < 0)
	{
		perror("binding socket failed\n");
		exit(1);
	}
	// _____________________________bind the socket and error check


	// _____________________________listen for connection
	if (listen( shell_socket,  3)<0)
	{
		perror("Listening to socket failed\n");
		exit(1);
	}
	// _____________________________listen for connection


	// _____________________________accept conncetion
	int addrlen = sizeof(shell_address);	

	int client_socket;
	
	// accept the client socket's connection while specifying the current socket name, a pointer to the socket struct, it's size
	client_socket = accept(shell_socket, (struct sockaddr *) &shell_address, (socklen_t *) &addrlen);

	if(client_socket<0) {
		perror("accepting failed\n");
		exit(1);
	}
	// _____________________________accept conncetion


	// _____________________________executing the commands
	// allocate memories
	char * valid_commands[COMMANDS] = {"ls", "pwd", "mkdir", "rm", "cat", "find", "echo", "mv", "grep", "clear", "exit", "quit"}; // list of supported commands
	char * filename = (char *) malloc((MAX_ARG_LEN+1) * sizeof(char)); // store filename when I/O redirection

	char ** args = (char **) malloc(MAX_ARGS * sizeof(char*)); // allocate memory for argument list (https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc)
	for (int i = 0; i < MAX_ARGS; i++) {
		args[i] = malloc((MAX_ARG_LEN+1) * sizeof(char));

		if (args == NULL) {
			perror("Failed to allocate argument list.\n");
			exit(1);
		}
	}

	char buffer[MAX_BUF];
	char output[MAX_OUTPUT];

	// _____________________________recieve user input from client socket and store in buffer
	while (1) { // repeat


		bzero(buffer, MAX_BUF);
		recv(client_socket, &buffer, sizeof(buffer), 0);
		printf("Buffer received: %s\n", buffer);
	// _____________________________recieve user input from client socket and store in buffer


		// _____________________________execute the command and send client socket the output
		// using pipes to redirect output of exec (https://stackoverflow.com/questions/2605130/redirecting-exec-output-to-a-buffer-or-file)
		int fd[2];
		pipe(fd);
		pid_t pid = fork();
		if (pid == 0) { // child
			close(fd[0]); // close reading end
			dup2(fd[1], STDOUT_FILENO); // send stdout to the pipe
			dup2(fd[1], STDERR_FILENO); // send stderr to the pipe
			close(fd[1]);
			
			if (is_empty(buffer) == false) // if the buffer is not empty
			{
				// handle I/O redirection
				bool redirect_input_found = false; // to check if there is any I/O redirection
				bool redirect_output_found = false;
				int default_fd; // to restore default stdin/stdout later
				filename = check_if_io_redirection(buffer, &redirect_input_found, &redirect_output_found); // check if there is any input/output redirection, if there is, return the filename & update the buffer (remove redirection sign and filename from the buffer)
				
				if (redirect_input_found == true && filename != NULL) { // if input_sign found, redirect input & return the default_fd 
					default_fd = redirect_input(filename);
				}
				else if (redirect_output_found == true && filename != NULL) { // else if output_sign found, redirect output & return the default_fd 
					default_fd = redirect_output(filename);
				}

				// handle pipes
				int pipe_num = check_pipes(buffer); // check if there is any pipe and if there is, return the number of pipes
				if (pipe_num > 3) { // error if more than 3 pipes
					perror("Only 1 to 3 pipes are supported.\n");
				}
				else if (pipe_num > 0) { // if 1 - 3 pipe exists 
					char * divided_buffers[pipe_num + 1]; // create a new buffer array for each separate command
					divide_buffer(buffer, divided_buffers, pipe_num); // divide buffer and store each command into the divided_buffers array
					execute_pipes(args, valid_commands, divided_buffers, pipe_num); // execute each command in the divided_buffers
				}
				else { // if no pipe
					get_argument_list(buffer, args); // divide user input and store each argument into an argument list
					execute(args, valid_commands);
				}

				// restore the default stdin and stdout if I/O redirection happened
				if (redirect_input_found == true) {
					dup2(default_fd, STDIN_FILENO);
					close(default_fd);
				}
				else if (redirect_output_found == true) {
					dup2(default_fd, STDOUT_FILENO);
					close(default_fd);
				}
			}
			exit(0);
		}
		else if (pid > 0) { // parent
			wait(NULL);
			close(fd[1]);
			bzero(output, 1024);
			read(fd[0], output, sizeof(output));
			close(fd[0]);
			dup2(1, STDOUT_FILENO); // restore default fd

			// _____________________________send client socket the output of command
			send(client_socket, output, sizeof(output), 0);
			//// _____________________________

			
			if (strcmp(output, "EXIT") == 0) { // if exit
				break;
			}
		}
	}
	// _____________________________execute the command and send client socket the output

	// _____________________________executing the commands

	//close the sockets
	close(shell_socket);
	close(client_socket);

	return 0;
}