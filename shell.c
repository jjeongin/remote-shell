// Remote shell project [Phase 1] 
// Created Date: Feb 26, 2022
// Latest Update : Mar 11, 2022
// Author: Jeongin Lee

// Command List : ls, clear, pwd, mkdir, cat, echo, find, mv, rm, cd

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "command.h"
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument
#define COMMANDS 10

int main(void) {
	// --STATIC ALLOCATION--
	// char buffer[MAX_BUF]; // string to store the user input
	// char * args[MAX_ARGS]; // list of string to store the arguments
	int arg_len;
	char * valid_commands[COMMANDS] = {"ls", "pwd", "clear", "mkdir", "rm", "cd", "cat", "find", "echo", "mv"}; // list of supported commands
	char * filename = (char *) malloc((MAX_ARG_LEN+1) * sizeof(char));

	// --DYNAMIC ALLOCATION--
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

 		if(args == NULL) {
			perror("Failed to allocate argument list.\n");
			exit(1);
		}
	}

	printf("---Welcome to the shell---\n");
	while(1) { // repeat while the command is not exit or quit
		arg_len = 0; // number of arguments

		// shell interface
		char * cwd = getcwd(NULL, 0); // print current working directory
		printf("%s $ ", cwd);

		// get user input and store it in argument list
		get_user_input(buffer, bufsize); // get user input and store it in buffer

		// handle input/output redirection
		bool redirect_input_found = false;
		bool redirect_output_found = false;
		int default_fd;
		filename = check_if_io_redirection(buffer, &redirect_input_found, &redirect_output_found); // check if there is input/output redirection, if there is, return the redirect sign & filename and update buffer (parse the input string)
		// TEST
		// printf("New buffer in shell.c : \"%s\"\n", buffer);
		// printf("Filename in shell.c : \"%s\"\n", filename);

		if (redirect_input_found == true && filename != NULL) { // if input_sign found, redirect input & return the default_fd 
			default_fd = redirect_input(filename);
		}
		else if (redirect_output_found == true && filename != NULL) { // else if output_sign found, redirect output & return the default_fd 
			default_fd = redirect_output(filename);
		}

		// TO DO - check if pipe sign ("|") in the string
		// check if pipe and return the number of the pipe detected
		// if there is at least one pipe, divide the input into multiple string
		// loop through each string, divide each into args list, and execute each

		arg_len = get_argument_list(buffer, args); // divide user input into argument list 

		if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0) // end the program if the command is exit or quit
			return 0;

		if (check_if_valid_command(args[0], valid_commands) == false) { // error if the command is not in valid command list
			printf("Invalid command : \"%s\"\n", args[0]);
		}
		else { // else, execute the program
			// execute arguments in the args list - FOR PIPE : Maybe we can loop this line if there is multiple arguments that needs to be executed
			execute(args);

			// restore default stdin and stdout
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

	// --DYNAMIC MEMORY--
	free(filename);
	free(buffer);
	for (int i = 0; i < sizeof(args)/sizeof(args[0]); i++) // free each string in argument list
		free(args[i]);
	free(args);
	return 0;
}