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
#define COMMANDS 13

void get_user_input(char * buffer, size_t bufsize) {
	getline(&buffer, &bufsize, stdin);
	buffer[strcspn(buffer, "\n")] = 0; // remove new line character at the end (https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input)
	// TEST
	// printf("Buffer : \"%s\"\n", buffer);
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
	if (temp_fd < 0) { // exit with error if filename not found
		printf("Failed to open file with filename : \"%s\"\n", filename);
		exit(1);
	}
	dup2(temp_fd, STDIN_FILENO);
	close(temp_fd);
	return default_fd;
}

int redirect_output(char * filename) {
	int default_fd = dup(STDOUT_FILENO);
	int temp_fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
	if (temp_fd < 0) { // exit with error if filename not found
		printf("Failed to open file with filename : \"%s\"\n", filename);
		exit(1);
	}
	dup2(temp_fd, STDOUT_FILENO);
	close(temp_fd);
	return default_fd;
}

char * check_if_io_redirection(char * buffer, bool * redirect_input_found, bool * redirect_output_found) {
	char * redirect_input_sign = strstr(buffer, "<");
	char * redirect_output_sign = strstr(buffer, ">");
	char * filename = (char *) malloc((MAX_ARG_LEN+1) * sizeof(char));
	char * delim = "\t\r\n ";
	int pos = 0;

	if (redirect_input_sign != NULL) { // if input redirection sign is found
		*redirect_input_found = true; // set signal to true
		strcpy(filename, redirect_input_sign + 1); // copy the filename
		filename = strtok(filename, delim); // remove any whitespace from the filename
		pos = redirect_input_sign - buffer; // position (index) of the redirection sign
	}
	else if (redirect_output_sign != NULL) { // if output redirection sign is found
		*redirect_output_found = true;
		strcpy(filename, redirect_output_sign + 1);
		filename = strtok(filename, delim);
		pos = redirect_output_sign - buffer; // position (index) of the redirection sign
	}

	if (pos > 0) { // if any sign is found
		for (int i = strlen(buffer)-1; i >= pos; i--) { // remove the redirect sign and filename from the back of the buffer
			buffer[i] = '\0';
		}
	}

	return filename;
}

int execute(char ** args) { // execute the command
	int status;

	if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0) // end the program if the command is exit or quit
		exit(0);
	else if (strcmp(args[0], "cd") == 0) { // if cd, execute chdir()
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

// check if there is any pipe and return the number of pipes
int check_pipes(char * buffer)
{
	int pipe_num = 0;
	for(int i = 0 ; i < strlen(buffer) ; i++)
	{
		if (buffer[i]== '|')
		    pipe_num++;
	}
	return pipe_num;
}

// divide buffer into pipe lists & execute each args
void divide_buffer(char * buffer, char ** divided_buffers, int pipe_num)
{
	int buffer_num = 0;
	//code to divide user input derived from https://stackoverflow.com/questions/15472299/split-string-into-tokens-and-save-them-in-an-array
	if (pipe_num < 4){ // only execute upto three pipes
		divided_buffers[buffer_num] = strtok(buffer, "|");
		printf("divided_buffers %d : %s\n", buffer_num, divided_buffers[buffer_num]);
		while (divided_buffers[buffer_num] != NULL)	//we then itterate over the charecters until the pointer and append the chars to each respective array 
		{
			buffer_num++;
			divided_buffers[buffer_num] = strtok(NULL, "|");
			printf("divided_buffers %d : %s\n", buffer_num, divided_buffers[buffer_num]);
		}
	}
	else {
		perror("Only 1 to 3 pipes are supported.\n");
	}
}

int execute_pipes(char ** current_args, char ** divided_buffers, int pipe_num)
{
	int fd[2];
	pid_t pid;
	int counter = 0;
	int fd_in = 0;
	int default_fd_in = dup(STDIN_FILENO); // save default stdin
	int default_fd_out = dup(STDOUT_FILENO); // save default stdout

	// while loop logic derived from https://stackoverflow.com/questions/17630247/coding-multiple-pipe-in-c
	while(counter < pipe_num + 1) {
		get_argument_list(divided_buffers[counter], current_args); // update current arguments with the next buffer

		if (pipe(fd) < 0)
		{
			perror("Failed to create pipe.\n");
			exit(EXIT_FAILURE);
		}
		pid = fork();
		if (pid < 0)
		{
			perror("Failed to fork.\n"); // fork failed
			exit(1);
		}
		else if (pid == 0) // child
		{
			dup2(fd_in, STDIN_FILENO); // redirect stdin to the writing end of the pipe
			if (counter + 1  < pipe_num + 1) // if there are next arguments 
				dup2(fd[1], STDOUT_FILENO); // redirect stdout to the reading end of the pipe
			close(fd[0]); // close the reading end of the pipe
			execute(current_args); // execute current arguments
	        exit(0);
		}
		else if (pid > 0) // parent
		{
			wait(NULL); // wait for the child to execute the first arguments
			close(fd[1]); // close the writing end of the pipe
			fd_in = fd[0]; // store fd in for the execution of the next arguments
			counter++; // increase counter
		}
	}
	dup2(default_fd_out, STDOUT_FILENO); // restore the default stdout
	dup2(default_fd_in, STDIN_FILENO); // restore the default stdin
	close(default_fd_out);
	close(default_fd_in);

	return 0;
}