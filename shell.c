// Remote shell project [Phase 1] 
// Date: Feb 26, 2022
// Author: Jeongin Lee

// -- SUGGESTION : grep instead of clear ? --
// Command list : ls, pwd, clear / mkdir, rm, cd, cat, find, echo, mv
// L commands with no arg -- ls, pwd, clear
// L commands with arg -- mkdir, rm, cd, cat, find, echo, mv
// L commands that can be used with I/O redirecting -- echo, cat, grep
// L pipes

// -- UPDATE 03/02 --
// Implemented commands using exec : ls, clear, pwd, mkdir, cat, echo, find, mv
// Implemented commands with extra methods : cd (new)
// TO DO : rm
//         I/O redirection
//         pipes

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "command.h"
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument
#define COMMANDS 10

int main(void) {
	// --STATIC ALLOCATION--
	char buffer[MAX_BUF]; // string to store the user input
	char * args[MAX_ARGS]; // list of string to store the arguments
	int arg_len;
	char * valid_commands[COMMANDS] = {"ls", "pwd", "clear", "mkdir", "rm", "cd", "cat", "find", "echo", "mv"}; // list of supported commands
	// char * exec_commands[] = 

	// --DYNAMIC ALLOCATION--
	// char * buffer; // allocate memory for buffer
	// size_t bufsize = MAX_BUF;
	// buffer = (char *) malloc(bufsize * sizeof(char));
	// if (buffer == NULL) {
	// 	perror("Failed to allocate buffer.\n");
	// 	exit(1);
	// }

	// char ** args = (char **) malloc(MAX_ARGS * sizeof(char*)); // allocate memory for argument list (https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc)
	// for (int i = 0; i < MAX_ARGS; i++)
 	//	args[i] = malloc((MAX_ARG_LEN+1) * sizeof(char));

 	//	if(args == NULL) {
	// 	perror("Failed to allocate argument list.\n");
	// 	exit(1);
	// }

	printf("---Welcome to the shell---\n");

	while(1) { // repeat while
		// buffer = get_input();
		arg_len = main_shell(buffer, args);
		if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0)
			return 0;

		execute(args, valid_commands);
	}
	// --DYNAMIC MEMORY--
	// free(buffer);
	// free(args);
	return 0;
}