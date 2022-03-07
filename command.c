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
#include <fcntl.h>

#include "command.h"
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument
#define COMMANDS 10

void get_user_input(char * buffer, size_t bufsize) {
	getline(&buffer, &bufsize, stdin);
	buffer[strcspn(buffer, "\n")] = 0; // remove new line character at the end (https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input)
	// TEST
	printf("Buffer : \"%s\"\n", buffer);
}

int get_argument_list(char * buffer, char ** args) {
	char * delim = "\t\r\n ";
	int arg_len = 0;

	args[arg_len] = strtok(buffer, delim); // first argument
	// TEST
	printf("Arg %d : \"%s\"\n", arg_len, args[arg_len]);
	while (args[arg_len] != NULL) {
		arg_len++;
		args[arg_len] = strtok(NULL, delim);
		// TEST
		printf("Arg %d : \"%s\"\n", arg_len, args[arg_len]);
	}
	// TEST
	printf("Arg length : %d.\n", arg_len);

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

int redirect_input(char * filename) {
	int default_fd = dup(STDIN_FILENO);
	int temp_fd = open(filename, O_RDONLY);
	if (temp_fd < 0) // error checking needed : check if filename exists
		// error
	dup2(temp_fd, STDIN_FILENO);
	close(temp_fd);
	return default_fd;
}

int redirect_output(char * filename) {
	int default_fd = dup(STDOUT_FILENO);
	int temp_fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
	dup2(temp_fd, STDOUT_FILENO);
	close(temp_fd);
	return default_fd;
}

char * check_if_io_redirection(char * buffer, size_t bufsize, bool redirect_input_found, bool redirect_output_found) {
	char * filename;
	char * redirect_input_sign = strchr(buffer, "<");
	char * redirect_output_sign = strchr(buffer, ">");
	char * delim = "\t\r\n ";
	int pos = 0;

	if (redirect_input_sign != NULL) {
		redirect_input_found = true;
		filename = buffer[redirect_input_sign+1]; // redirect input to the filename
		filename = strtok(filename, delim); // remove white space in filename
		int pos = redirect_input_sign - buffer;
	}
	else if (redirect_output_sign != NULL) {
		redirect_output_found = true;
		filename = buffer[redirect_output_sign+1]; // redirect input to the filename
		filename = strtok(filename, delim);// remove white space in filename
		int pos = redirect_output_sign - buffer;
	}

	if (pos > 0) {
		for (int i = pos; i < bufsize; i++) { // remove the redirect sign and filename from the back of the buffer
			buffer[i] = '\0';
		}
	}

	return filename;
}

// bool check_if_redirect_io(char ** args, int arg_len) {
// 	bool redirect_input_found = false;
// 	bool redirect_output_found = false;
// 	int i = 0;
// 	while (args[i] != NULL) {
// 		if (strcmp(args[i], "<") == 0) {
// 			redirect_input_found = true;
// 			filename = args[i+1]; // redirect input to the filename
// 			arg_len -= 2; // remove "<" and "filename" from the args list
// 		}
// 		else if (strcmp(args[i], ">") == 0) {
// 			redirect_output_found = true;
// 			filename = args[i+1]; // redirect input to the filename
// 			arg_len -= 2; // remove "<" and "filename" from the args list
// 		}

// 		if (redirect_input_found || redirect_output_found) // move the elements behind to the front
// 			args[i] = args[i+2];

// 		i++;
// 	}
// }

int execute(char ** args) { // execute the command
	int status;

	if (strcmp(args[0], "cd") == 0) { // if cd, execute chdir()
		if (chdir(args[1]) != 0) {
			printf("cd failed to %s\n", args[1]);
		}
	}
	else {
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