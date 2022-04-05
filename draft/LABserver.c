// to check:
	// pass string through with the commands
	//apply functions here
	//
// https://explainshell.com/explain?cmd=find%20.%20-type%20f%20-exec%20ls%20-s%20%7B%7D%20%5C;%20%7C%20sort%20-n%20-r%20%7C%20head.1posix%20-5
// https://www.unix.com/programming/88529-how-run-linux-command-redirect-its-output-socket-c.html
// socket
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



// _____________________
#define PORT 9002 //or 8080 or any other unused port value
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument
#define COMMANDS 13 // number of possible commands
#define _GNU_SOURCE
// _____________________


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
	if (bind(server_socket , 
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

	if(client_socket<0){
		printf("accept failed..\n");
		exit(EXIT_FAILURE);
	}
	


	// next steps:
		// pipe the output or store it into a string, then send it to client
		// loop over the whole thing


	char * valid_commands[COMMANDS] = {"ls", "pwd", "mkdir", "rm", "cd", "cat", "find", "echo", "mv", "grep", "clear", "exit", "quit"}; // list of supported commands
	//receive command message from client
	char command[1024];
	recv(client_socket , &command , sizeof(command),0);

	// _____________________
	// prepare the command
	char ** args = (char **) malloc(MAX_ARGS * sizeof(char*)); // allocate memory for argument list (https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc)
	for (int i = 0; i < MAX_ARGS; i++) {
		args[i] = malloc((MAX_ARG_LEN+1) * sizeof(char));

		if (args == NULL) {
			perror("Failed to allocate argument list.\n");
			exit(1);
		}
	}

	// # # # # # # # # pipes added

	int pipefds[2];
	pid_t pid;
	char buf[500];


	//create pipe
	if(pipe(pipefds) == -1){
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	//fill a block of memory with given/particular value.
	//fills the 30 blocks of buf with 0
		memset(buf,0,500);

		pid = fork();

	//PARENT 
	if (pid > 0) 
	{
		printf("PARENT write in pipe\n");

		//parent close the read end
		close(pipefds[0]);

		//parent write in the pipe write end                 
		dup2(pipefds[1],STDOUT_FILENO); 

		// execute the command
		get_argument_list(command, args); // divide user input and store each argument into an argument list
		execute(args, valid_commands); // execute arguments in the argument list

		//after finishing writing, parent close the write end
		close(pipefds[1]);

		//parent wait for child                
		wait(NULL); 

	}

	else{
		dup2(pipefds[0],STDIN_FILENO);
		close(pipefds[1]);         
		// while(read(pipefds[0], buf, 1)==1)   
		// 	printf("CHILD read from pipe -- %s\n", buf);
		read(pipefds[0],buf,sizeof(buf));
		// printf("\n%s\n\n",buf);
		send(client_socket , buf , sizeof(buf),0);
		close(pipefds[0]);
		printf("CHILD: EXITING!");
		exit(EXIT_SUCCESS);

	}




	//print out the data we get back
	// _____________________

	printf("Received: %s\n" , command);


	// pipeString[];
	// send(client_socket , pipeString , sizeof(pipeString),0);


	//close the socket
	close(server_socket); 



	return 0;
}