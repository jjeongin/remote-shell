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

#define PORT 9002 //or 8080 or any other unused port value
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument
#define COMMANDS 13 // number of possible commands
#define PORT 9002


int main()
{
	//create socket
	int server_socket;
	server_socket = socket(AF_INET , SOCK_STREAM,0);

	//check for fail error
	if (server_socket == -1) {
        printf("socket creation failed..\n");
        exit(EXIT_FAILURE);
    }
    
	//define server address structure
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;


	//bind the socket to our specified IP and port
	if (bind(server_socket, 
		(struct sockaddr *) &server_address,
		sizeof(server_address)) < 0)
	{
		printf("socket bind failed..\n");
        exit(EXIT_FAILURE);
	}
	 

	//after it is bound, we can listen for connections
	if(listen(server_socket,5)<0){
		printf("Listen failed..\n");
        exit(EXIT_FAILURE);
	}
	 


	int addrlen = sizeof(server_address);	

	int client_socket;
	
	client_socket = accept(server_socket,
		(struct sockaddr *) &server_address,
		(socklen_t *) &addrlen);

	if(client_socket<0) {
		printf("accept failed..\n");
        exit(EXIT_FAILURE);
	}
	

	// # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #
	// # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #
	// # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #
		// next steps:
			// pipe the output or store it into a string, then send it to client
			// loop over the whole thing

    // allocate memories
    char * valid_commands[COMMANDS] = {"ls", "pwd", "mkdir", "rm", "cd", "cat", "find", "echo", "mv", "grep", "clear", "exit", "quit"}; // list of supported commands
	char * filename = (char *) malloc((MAX_ARG_LEN+1) * sizeof(char)); // store filename when I/O redirection

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

    // receive command message from client
    recv(client_socket , &buffer , sizeof(buffer), 0);
    // Redirect IO

    printf("Now in server..");
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
            execute(args, valid_commands); // execute arguments in the argument list
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
    // send();

    // char command[1024];
    // recv(client_socket , &command , sizeof(command),0);

    // execlp(command, command, NULL); 
    //print out the data we get back
    // printf("Received: %s\n" , command);


		// pipeString[];
    // send(client_socket , pipeString , sizeof(pipeString),0);
    
	// # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #
	// # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #
	// # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #  # # # # #


	//close the socket
	close(server_socket);
    close(client_socket);

    free(filename);
	free(buffer);
	for (int i = 0; i < sizeof(args)/sizeof(args[0]); i++) // free each string in argument list
		free(args[i]);
	free(args);
	return 0;
}