// Remote shell project [Phase 3] 
// Author : Jeongin Lee, Nouf Alabbasi
// Command List : ls, clear, pwd, mkdir, cat, echo, find, mv, rm, cd, grep

#define _GNU_SOURCE
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
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stddef.h>
#include <sys/queue.h>
#include "command.h"

#define PORT 8080 //or 8080 or any other unused port value
#define MAX_BUF 500 // max buffer size (https://www.geeksforgeeks.org/making-linux-shell-c/)
#define MAX_OUTPUT 1024
#define MAX_ARGS 100 // max number of arguments
#define MAX_ARG_LEN 50 // max length of one argument
#define COMMANDS 12 // number of possible commands

// GLOBAL VARIABLES
sem_t *new_client_added;
sem_t *program_running;

char * valid_commands[COMMANDS] = {"ls", "pwd", "mkdir", "rm", "cat", "find", "echo", "mv", "grep", "clear", "exit", "quit"}; // list of supported commands

struct Program {
	pid_t tid;
	int current; // current progress, program is completed when current == burst
	int burst; // burst time for this program

	int pipe_num; // 0 to 2
	char ** args; // command arguments
	char ** divided_buffers; // for pipe execution, maximum 2 pipes
	LIST_ENTRY(Program) pointers; // pointers for waiting queue from sys/queue.h
};

struct Program * create_program(pid_t tid, int burst, int pipe_num, char ** args, char ** divided_buffers){
	struct Program *p = (struct Program *)malloc(sizeof(struct Program));
	// struct Program *p = (struct Program *)malloc( sizeof(struct Program) + MAX_ARGS * sizeof(char*) + MAX_ARGS * (MAX_ARG_LEN+1) * sizeof(char) );
	p->tid = tid;
	p->current = 0;
	p->burst = burst;
	p->pipe_num = pipe_num;

	// allocate memory for argument list and divided buffer
	p->args = (char **) malloc(MAX_ARGS * sizeof(char*)); // allocate memory for argument list (https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc)
	for (int i = 0; i < MAX_ARGS; i++) {
		p->args[i] = malloc((MAX_ARG_LEN+1) * sizeof(char));

		if (p->args == NULL) {
			perror("Failed to allocate argument list.\n");
			exit(1);
		}
	}

	p->divided_buffers = (char **) malloc(MAX_ARGS * sizeof(char*)); // allocate memory for argument list (https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc)
	for (int i = 0; i < MAX_ARGS; i++) {
		p->divided_buffers[i] = malloc((MAX_ARG_LEN+1) * sizeof(char));

		if (p->divided_buffers == NULL) {
			perror("Failed to allocate argument list.\n");
			exit(1);
		}
	}

	// copy the argument list and divided buffer
	int i = 0;
	while(args[i] != NULL) {
		strcpy(p->args[i], args[i]);
		// printf("Arg %d : \"%s\"\n", i, p->args[i]); // TEST
		i++;
	}
	p->args[i] = NULL;

	int j = 0;
	while(divided_buffers[j] != NULL) {
		strcpy(p->divided_buffers[j], divided_buffers[j]);
		// printf("Arg %d : \"%s\"\n", i, p->args[i]); // TEST
		j++;
	}
	p->divided_buffers[j] = NULL;

	return p;
}

void* client_handler(void * socket);
void* scheduler(void * socket);
void schedule_waiting_queue();
void execute_program(struct Program * p); // execute each program

LIST_HEAD(head, Program); // create head of the waiting queue (doubly linked list of programs)
struct head waiting_queue;

int main()
{
	LIST_INIT(&waiting_queue); // initialize waiting queue

	sem_close(new_client_added); // reinitialize in case exited with error
   	sem_close(program_running);
   	sem_unlink("/new_client_added");
    sem_unlink("/program_running");

	new_client_added = sem_open("/new_client_added", O_CREAT, 0666, 1);
	program_running = sem_open("/program_running", O_CREAT, 0666, 1);

	// _____________________________socket code pt 1(socket code inspired by the code from lab 7)
	// create a socket
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


	int addrlen = sizeof(shell_address);	
	int client_socket;

	// _____________________________bind the socket to the local address and error check
	if (bind(shell_socket, (struct sockaddr *) &shell_address, sizeof(shell_address)) < 0)
	{
		perror("binding socket failed\n");
		exit(1);
	}
	// _____________________________bind the socket and error check

	while(1){
		// _____________________________listen for connection
		if (listen(shell_socket,  3) < 0)
		{
			perror("Listening to socket failed\n");
			exit(1);
		}
		// _____________________________listen for connection

		// _____________________________accept conncetion
		// accept the client socket's connection while specifying the current socket name, a pointer to the socket struct, it's size
		client_socket = accept(shell_socket, (struct sockaddr *) &shell_address, (socklen_t *) &addrlen);
		if(client_socket < 0) {
			perror("accepting failed\n");
			exit(1);
		}
		// _____________________________accept conncetion
		
		pthread_t scheduler_thread, client_thread;
		pthread_create(&scheduler_thread, NULL, scheduler, &client_socket);
		pthread_create(&client_thread, NULL, client_handler, &client_socket);
		// _____________________________create thread

		pthread_join(client_thread, NULL); // wait until the program is added to the waiting queue
		pthread_join(scheduler, NULL); // wait until the programs are scheduled
		
		printf("MAIN thread:\n");

		if(!LIST_EMPTY(&waiting_queue)) {
			struct Program *first = LIST_FIRST(&waiting_queue); // execute program from the top of the waiting queue
			execute_program(first);

			if (first->current == first->burst){  // if program finished being executed
				LIST_REMOVE(first, pointers);
			}
		}

		printf("Main thread executed\n");
	}

	struct Program *head = LIST_FIRST(&waiting_queue); // free waiting queue
   	while (head != NULL) {
       struct Program *next = LIST_NEXT(head, pointers);
       // TO DO ------------ also free argument list & divided buffers !
       free(head);
       head = next;
   	}
   	LIST_INIT(&waiting_queue);

   	sem_close(new_client_added);
   	sem_close(program_running);
   	sem_unlink("/new_client_added");
    sem_unlink("/program_running");

	close(shell_socket);
	return 0;
}

// ----------------------------------------------------------

void execute_program(struct Program * p){
	char output[MAX_OUTPUT];

	int pipe_num = p->pipe_num;
	char ** args = p->args;
	char ** divided_buffers = p->divided_buffers;

	int current = p->current;
	int burst = p->burst;
	int time = 0; // counter

	for (int i = current; i < burst; i++) {
		sem_wait(program_running);

		p->current = i; // update current progress for the program
		sleep(1);
		time++;
		printf("slept for %d/%ds\n", time, burst);

		sem_post(program_running);
	}
	p->current = p->burst;

	if (pipe_num > 0) {
		execute_pipes(args, valid_commands, divided_buffers, pipe_num);
	}
	else {
		execute(args, valid_commands);
	}
}

void * scheduler(void * socket){
	sem_wait(new_client_added);
	sem_wait(program_running);

	if(!LIST_EMPTY(&waiting_queue))
		schedule_waiting_queue();

	sem_post(new_client_added);
	sem_post(program_running);
}

void check_for_SJR(){ //return list
	// inspired by https://www.geeksforgeeks.org/bubble-sort-on-doubly-linked-list/
	// order list according to buble sort
	int swapped;
  	struct Program *head = LIST_FIRST(&waiting_queue); // free waiting queue
  	struct Program *current = head;
	struct Program *next = LIST_NEXT(current, pointers);

	// is the list empty?
	if (head == NULL)
		return;

	do
	{
		swapped = 0;
  
		while (current != NULL)     
		{
			if (current->burst > next->burst)
			{
				//check if this works
				struct Program *temp = current;
				current = next;
				next = next;
				swapped = 1;
			}
			current = next;
		}
	}
	while (swapped);
}


void schedule_waiting_queue(){
	printf("scheduling ...\n");
	sleep(5);

	// ERROR in scheduling ----------------
	// // reorganize the thread 
	// check_for_SJR();

	// int Q_time = 3; //quantum time
	// int G_time = 0; //general time
	// struct Program *head = LIST_FIRST(&waiting_queue); // free waiting queue

	// while(head != NULL)
	// {
	// 	//run code based on rr
	// 	struct Program *current = head; // free waiting queue
	// 	struct Program *next = LIST_NEXT(current, pointers);


	// 	bool flag = true;
	// 	while(current){
	// 		// if the process is not complete
	// 		if(current -> burst != 0)
	// 		{
	// 			// go into the critical state
	// 			flag = false;
	// 			break;
	// 		}
	// 		current = next;
	// 	}

	// 	// wait until an element is added
	// 	if(flag)
	// 		break;

	// 	// itterate though the head element until the qm is more than it's current time
	// 	do{
	// 		if (next != NULL) //if this is not the last element
	// 		{
	// 			int currentTimeProcess = 0;
	// 			while (currentTimeProcess<Q_time && current->burst>0)
	// 			{
	// 				sleep(1);
	// 				current->burst -= 1;
	// 				currentTimeProcess ++;
	// 				G_time +=1;
	// 			}
	// 		}

	// 		check_for_SJR(); //reorganize the list
	// 	}while(head != NULL);
	// }
	printf("scheduled !\n");
}

void* client_handler(void * socket){
	int *sock=(int*)socket;
	int s=*sock;

	// _____________________________executing the commands
	// allocate memories
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
	bzero(buffer, MAX_BUF);
	recv(s, &buffer, sizeof(buffer), 0);
	printf("Buffer received: %s\n", buffer);
	// _____________________________recieve user input from client socket and store in buffer
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
		char * divided_buffers[pipe_num + 1]; // create a new buffer array for each separate command
		divided_buffers[0] = NULL; // initialize to avoid segmentation fault
		if (pipe_num > 3) { // error if more than 3 pipes
			perror("Only 1 to 3 pipes are supported.\n");
		}
		else if (pipe_num > 0) { // if 1 - 3 pipe exists 
			divide_buffer(buffer, divided_buffers, pipe_num); // divide buffer and store each command into the divided_buffers array
			
			// printf("semaphore waiting ...\n");
			sem_wait(new_client_added); // lock the semaphore
			// printf("semaphore finished waiting !\n");

			// Add current program to waiting queue
			pid_t tid = pthread_self();
			int burst = 3;
			struct Program *program = create_program(tid, burst, pipe_num, args, divided_buffers);
			LIST_INSERT_HEAD(&waiting_queue, program, pointers);

			// Print all the elements in waiting queue to check if the program is added correctly
			struct Program *p;
			LIST_FOREACH(p, &waiting_queue, pointers) {
				printf("programs in waiting queue args[0]: %s\n", p->args[0]);
				printf("programs in waiting queue args[1]: %s\n", p->args[1]);
				printf("programs in waiting queue divided_buffers[0]: %s\n", p->divided_buffers[0]);
				printf("programs in waiting queue divided_buffers[1]: %s\n", p->divided_buffers[1]);
			}

			sem_post(new_client_added); // release the semaphore
			// printf("semaphore released !\n");
		}
		else { // if no pipe
			get_argument_list(buffer, args); // divide user input and store each argument into an argument list
			
			// printf("semaphore waiting ...\n");
			sem_wait(new_client_added); // lock the semaphore
			// printf("semaphore finished waiting !\n");

			// Add current program to waiting queue
			pid_t tid = pthread_self();
			int burst = 3;

			// example
			if (strcmp(args[0], "ls") == 0)
				burst = 2;
			else if (strcmp(args[0], "pwd") == 0)
				burst = 3;
			else if (strcmp(args[0], "echo") == 0)
				burst = 4;

			struct Program *program = create_program(tid, burst, pipe_num, args, divided_buffers);
			LIST_INSERT_HEAD(&waiting_queue, program, pointers);

			// Print all the elements in waiting queue to check if the program is added correctly
			struct Program *p;
			LIST_FOREACH(p, &waiting_queue, pointers) {
				printf("programs in waiting queue args[0]: %s\n", p->args[0]);
				printf("programs in waiting queue args[1]: %s\n", p->args[1]);
			}

			sem_post(new_client_added); // release the semaphore	
			// printf("semaphore released !\n");	
		}
	}

	//close the sockets
	close(s);
}
