// Remote shell project [Phase 2] 
// Created Date : Mar 28, 2022
// Latest Update : April 6, 2022
// Author : Jeongin Lee, Nouf Alabbasi
// Command List : ls, clear, pwd, mkdir, cat, echo, find, mv, rm, grep

// Shell Implementation

#define _GNU_SOURCE
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
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument
#define COMMANDS 13 // number of possible commands

int main(void) {
	// global variables
	char * valid_commands[COMMANDS] = {"ls", "pwd", "mkdir", "rm", "cat", "find", "echo", "mv", "grep", "clear", "exit", "quit"}; // list of supported commands
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

	printf("---Welcome to the shell---\n");
	while(1) { // repeat
		// shell interface
		char * cwd = getcwd(NULL, 0); // print current working directory
		printf("%s $ ", cwd);

		get_user_input(buffer, bufsize); // get user input and store it in the buffer
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
	}

	// free the allocated memories
	free(filename);
	free(buffer);
	for (int i = 0; i < sizeof(args)/sizeof(args[0]); i++) // free each string in argument list
		free(args[i]);
	free(args);
	return 0;
}