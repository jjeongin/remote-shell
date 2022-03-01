// Remote shell project [Phase 1] 
// Date: Feb 26, 2022
// Author: Jeongin Lee

// -- SUGGESTION : grep instead of clear ? --
// Command list : ls, pwd, clear / mkdir, rm, cd, cat, find, echo, mv
// L commands with no arg -- ls, pwd, clear
// L commands with arg -- mkdir, rm, cd, cat, find, echo, mv
// L commands that can be used with I/O redirecting -- echo, cat, grep
// L pipes

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "command.h"
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument

void main_shell(char * buffer, char ** args) {
	size_t bufsize = MAX_BUF;
	int arg_len = 0; // number of arguments

	while (arg_len <= 0) { // prevent segmentation fault when arg[0] == NULL
		printf("> ");
		getline(&buffer, &bufsize, stdin);
		buffer[strcspn(buffer, "\n")] = 0; // remove new line character at the end (https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input)
		printf("Buffer : \"%s\"\n", buffer);

		args[arg_len] = strtok(buffer, " "); // first argument
		printf("Arg %d : \"%s\"\n", arg_len, args[arg_len]);
		while (args[arg_len] != NULL) {
			arg_len++;
			args[arg_len] = strtok(NULL, " ");
			printf("Arg %d : \"%s\"\n", arg_len, args[arg_len]);
		}
		printf("Arg length : %d.\n", arg_len);
	}
}

int main(void) {
	// --STATIC MEMORY--
	char buffer[MAX_BUF];
	char * args[MAX_ARGS];

	// --DYNAMIC MEMORY--
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
		main_shell(buffer, args); // main shell
		if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0)
			return 0;

		pid_t pid = fork();
		if (pid < 0) { // fork error
			perror("Failed to Fork.\n");
			return 1;
		}
		else if (pid == 0) { // child
			printf("Child 1\n");
			// execute command
			exit(0);
		}

		wait(NULL);
		printf("Parent\n");
	}
	// --DYNAMIC MEMORY--
	// free(buffer);
	// free(args);
	return 0;
}