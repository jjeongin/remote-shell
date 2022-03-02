// Remote shell project [Phase 1] 
// Date: Feb 26, 2022
// Author: Jeongin Lee

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

int main_shell(char * buffer, char ** args) { // main shell interface
	size_t bufsize = MAX_BUF;
	int arg_len = 0; // number of arguments
	char * delim = "\t\r\n ";

	while (arg_len <= 0) { // repeat until user enter at least one argument
		char * cwd = getcwd(NULL, 0); // current working directory
		printf("%s > ", cwd);
		getline(&buffer, &bufsize, stdin);
		buffer[strcspn(buffer, "\n")] = 0; // remove new line character at the end (https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input)
		printf("Buffer : \"%s\"\n", buffer);

		args[arg_len] = strtok(buffer, delim); // first argument
		printf("Arg %d : \"%s\"\n", arg_len, args[arg_len]);
		while (args[arg_len] != NULL) {
			arg_len++;
			args[arg_len] = strtok(NULL, delim);
			printf("Arg %d : \"%s\"\n", arg_len, args[arg_len]);
		}
		printf("Arg length : %d.\n", arg_len);
	}
	return arg_len;
}

bool check_if_valid_command(char * command, char ** valid_commands) { // check if the command is in valid command list
	for (int i = 0; i < COMMANDS; i++) {
        if (strcmp(command, valid_commands[i]) == 0) {
            return true;
        }
    }
    return false;
}

int execute(char ** args, char ** valid_commands) { // execute the command
	int status;

	if (check_if_valid_command(args[0], valid_commands) == false) { // if the command is not in valid command list
		printf("Invalid command: %s\n", args[0]); // print error and return
		return 1;
	}

	if (strcmp(args[0], "cd") == 0) { // run change directory
		if (chdir(args[1]) != 0) {
			printf("cd failed to %s\n", args[1]);
		}
	}
	else { // for other commands that can be executed using exec()
		pid_t pid = fork();
	    if (pid < 0) // proccess failed
	    {
	        perror("Failed to Fork.\n");
	        exit(1);
	    }
	    else if (pid == 0) // child
	    {
	        if (execvp(args[0], args) < 0) { // execute the command with argument list
	            perror("Falied to execute the command.\n"); // if execution failed
	            exit(1);
	        }
	        exit(0);
	    }
	    else { // parent
	    	waitpid(pid, &status, 0);
	    }
	}

    return 0;
}

// Should we separate extra commands ?
// int remove_file() {

// }

// int change_directory(char ** args) {
// 	if (chdir(args[1]) != 0) {
// 		printf("cd failed to %s\n", args[1]);
// 	}
// }