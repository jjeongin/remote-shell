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

void* client_handler(void * socket);
sem_t *sem_wq; // declare semaphore
sem_t *sem_running;
sem_t *sem_not_running;

char * valid_commands[COMMANDS] = {"ls", "pwd", "mkdir", "rm", "cat", "find", "echo", "mv", "grep", "clear", "exit", "quit"}; // list of supported commands

struct Program {
	pid_t tid;
	int socket;
	int current; // current progress, program is completed when current == burst
	int burst; // burst time for this program

	int pipe_num; // 0 to 2
	char ** args; // command arguments
	char ** divided_buffers; // for pipe execution, maximum 2 pipes
	LIST_ENTRY(Program) pointers; // pointers for waiting queue from sys/queue.h
};

struct Program * create_program(pid_t tid, int socket, int burst, int pipe_num, char ** args, char ** divided_buffers){
	struct Program *p = (struct Program *)malloc(sizeof(struct Program));
	// struct Program *p = (struct Program *)malloc( sizeof(struct Program) + MAX_ARGS * sizeof(char*) + MAX_ARGS * (MAX_ARG_LEN+1) * sizeof(char) );
	p->tid = tid;
	p->socket = socket;
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

	// for (int j = 0; j < pipe_num + 1; j++) {
	// 	p->divided_buffers[j] = malloc((MAX_ARG_LEN+1) * sizeof(char));
	// }

	// p->divided_buffers = (char **) malloc((pipe_num+1) * sizeof(char*)); // allocate memory for argument list (https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc)
	// for (int i = 0; i < pipe_num+1; i++) {
	// 	p->args[i] = malloc((MAX_ARG_LEN+1) * sizeof(char));

	// 	if (p->args == NULL) {
	// 		perror("Failed to allocate argument list.\n");
	// 		exit(1);
	// 	}
	// }

	// copy the argument list and divided buffer
	int i = 0;
	while(args[i] != NULL) {
		strcpy(p->args[i], args[i]);
		// printf("Arg %d : \"%s\"\n", i, p->args[i]); // TEST
		i++;
	}
	p->args[i] = NULL;

	// memcpy(p->divided_buffers, divided_buffers, sizeof(divided_buffers));

	// for (int j = 0; j < pipe_num + 1; j++) {
	// 	p->divided_buffers[j] = divided_buffers[j];
	// }
	// p->divided_buffers[pipe_num+1] = '\0';

	// int j = 0;
	// while(divided_buffers[j] != NULL) {
	// 	strcpy(p->divided_buffers[j], divided_buffers[j]);
	// 	j++;
	// }

	return p;
}

void* scheduler(void * socket, struct Program * p);
void check_for_SJR(void * socket, struct Program * p);
int execute_program(struct Program * p); // execute each program

LIST_HEAD(head, Program); // create head of the waiting queue (doubly linked list of programs)
struct head waiting_queue;

int main()
{
	LIST_INIT(&waiting_queue); // initialize waiting queue
	
	// sem_unlink(sem_wq);
	// sem_unlink(sem_running);
	// sem_unlink(sem_not_running);

	if ((sem_wq = sem_open("/semaphore_wq", O_CREAT, 0666, 1)) == SEM_FAILED) {
	    perror("sem_open\n");
	    exit(1);
	}

	if ((sem_running = sem_open("/semaphore_running", O_CREAT, 0666, 1)) == SEM_FAILED) {
	    perror("sem_open\n");
	    exit(1);
	}

	if ((sem_not_running = sem_open("/semaphore_not_running", O_CREAT, 0666, 0)) == SEM_FAILED) {
	    perror("sem_open\n");
	    exit(1);
	}

	// sem_wq = sem_open("/semaphore_wq", O_CREAT,  0666, 1); // initialize semaphore
	// int sem_val;
	// printf("semaphore value, %d\n", sem_getvalue(sem_wq, &sem_val));

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

		struct Program *first = LIST_FIRST(&waiting_queue); // execute program from the top of the waiting queue
		execute_program(first);

		if (first->current == first->burst) // if program finished being executed
			LIST_REMOVE(first, pointers);
		else // else add it back to the waiting queue
			LIST_INSERT_HEAD(&waiting_queue, first, pointers);

		printf("Main thread executed\n");

		// struct Program *p;
		// LIST_FOREACH(p, &waiting_queue, pointers) {
		// 	printf("[MAIN] programs in waiting queue args[0]: %s\n", p->args[0]);
		// 	printf("[MAIN] programs in waiting queue args[1]: %s\n", p->args[1]);
		// }

	}

	struct Program *head = LIST_FIRST(&waiting_queue); // free waiting queue
// <<<<<<< HEAD
// 	while (head != NULL) {
// 	   struct Program *next = LIST_NEXT(head, pointers);
// 	   free(head);
// 	   head = next;
// 	}
// 	LIST_INIT(&waiting_queue);

// 	sem_close(sem_wq); // close semaphore
// 	sem_unlink("sem_wq");
// =======
   	while (head != NULL) {
       struct Program *next = LIST_NEXT(head, pointers);
       // free argument list & divided buffers
       free(head);
       head = next;
   	}
   	LIST_INIT(&waiting_queue);

   	sem_close(sem_wq); // close semaphore
   	sem_close(sem_running); // close semaphore
   	sem_close(sem_not_running); // close semaphore
    sem_unlink("/semaphore_wq");
    sem_unlink("/semaphore_running");
    sem_unlink("/semaphore_not_running");
// >>>>>>> 68071df6cb437f586ebdf7c5851787e443b140bf

	close(shell_socket);
	return 0; 
}

// ----------------------------------------------------------

int execute_program(struct Program * p){
	char output[MAX_OUTPUT];
	int s = p->socket; // client socket

	int pipe_num = p->pipe_num;
	char ** args = p->args;
	char ** divided_buffers = p->divided_buffers;

	int current = p->current;
	int burst = p->burst;
	int time = 0; // counter


	printf("semaphore running waiting ...\n");
	sem_wait(sem_running); // we can use multiple semaphores instead of checking semaphore value (sem_get_value() deprecated in MAC)
	printf("semaphore wq waiting ...\n");
	sem_wait(sem_wq);
	printf("semaphores finished waiting !\n");

	if (pipe_num > 0) {
		execute_pipes(args, valid_commands, divided_buffers, pipe_num);
	}
	else {
		execute(args, valid_commands);
	}

	// execute the command and send client socket the output
	// using pipes to redirect output of exec (https://stackoverflow.com/questions/2605130/redirecting-exec-output-to-a-buffer-or-file)
	int fd[2];
	pipe(fd);
	pid_t pid = fork();
	if (pid == 0) { // child
		close(fd[0]); // close reading end
		dup2(fd[1], STDOUT_FILENO); // send stdout to the pipe
		dup2(fd[1], STDERR_FILENO); // send stderr to the pipe
		close(fd[1]);

		if (pipe_num > 0) {
			execute_pipes(args, valid_commands, divided_buffers, pipe_num);
		}
		else {
			execute(args, valid_commands);
		}
		exit(0);
	}
	else if (pid > 0) { // parent
		wait(NULL);
		close(fd[1]);
		bzero(output, 1024);
		read(fd[0], output, sizeof(output));
		close(fd[0]);

		for (int i = current; i <= burst; i++) {
			p->current = i; // update current progress for the program

			sleep(1);
			time++;
			printf("slept for %d/%ds\n", time, burst);
		}

		printf("output: %s\n", output);

		// _____________________________send back to client
		dup2(1, STDOUT_FILENO); // restore default fd

		// // _____________________________send client socket the output of command // apply DELAY here
		// for (int i = current; i <= burst; i++) {
		// send(s, output, sizeof(output), 0); // not sure if this will work ?
		// }
		// //// _____________________________

		// if (strcmp(output, "EXIT") == 0) { // if exit // how can we exit the client now ?
		// 	break;
		// }
	}

	p->current = burst; // program finished executing

	// release semaphore
	sem_post(sem_wq);
	sem_post(sem_running);
	printf("semaphores released !\n");
	
	// close(s);

	return burst;
}

void* scheduler(void * socket, struct Program * p){
	// reorder list based on the burst time
	// reorganize the thread 
	check_for_SJR();

	int Q = 3; //quantum
	int t = 0; //time

	while(head == NULL)
	{
		//run code based on rr

		bool flag = true;

		struct node *crrNODE;
		crrNODE = head;		//check(head or p)

		for(int i = 0; i < n; i++){
			if(crrNODE->burst != 0){
				flag = false;
				break;
			}
			crrNODE = crrNODE->next;
		}
		if(flag)
			break;

		for(int i = 0; (i < n) && (head == NULL); i++){
			int ctr = 0;
			while((ctr < tq) && (temp_burst[queue[0]-1] > 0)){
				sleep(1);
				temp_burst[queue[0]-1] -= 1;
				timer += 1;
				ctr++;

				//Checking and Updating the ready queue until all the processes arrive
				checkNewArrival(timer, arrival, n, maxProccessIndex, queue);
			}
			//If a process is completed then store its exit time
			//and mark it as completed
			if((temp_burst[queue[0]-1] == 0) && (complete[queue[0]-1] == false)){
				//turn array currently stores the completion time
				turn[queue[0]-1] = timer;	
				complete[queue[0]-1] = true;
			}
			
			//checks whether or not CPU is idle
			bool idle = true;
			if(queue[n-1] == 0){
				for(int i = 0; i < n && queue[i] != 0; i++){
					if(complete[queue[i]-1] == false){
						idle = false;
					}
				}
			}
			else
				idle = false;

			if(idle){
				timer++;
				checkNewArrival(timer, arrival, n, maxProccessIndex, queue);
			}
	
			//Maintaining the entries of processes
			//after each premption in the ready Queue
			queueMaintainence(queue,n);
		}
	}
	
	// reorganize the thread (maintain sjrf)
	check_for_SJR();

}

void check_for_SJR(void * socket, struct Program * p){
	// inspired by https://www.geeksforgeeks.org/bubble-sort-on-doubly-linked-list/
	// order list according to bubble sort
	int swapped;
	struct node *currentptr = NULL;
  
	// empty list?
	if (head == NULL)
		return;
  
	do
	{
		swapped = 0;
		p = head;
  
		while (p->next != currentptr)     
		{
			if (p->data > p->next->data)
			{
				//check if this works
				program* temp = p;
				p = p -> next;
				p -> next = temp;

				swapped = 1;
			}
			p = p->next;
		}
		currentptr = p;
	}
	while (swapped);
}


void* client_handler(void * socket){
	int *sock=(int*)socket;
	int s=*sock;

	// _____________________________executing the commands
	// allocate memories
	// char * valid_commands[COMMANDS] = {"ls", "pwd", "mkdir", "rm", "cat", "find", "echo", "mv", "grep", "clear", "exit", "quit"}; // list of supported commands
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
	// while (1) { // repeat
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
		if (pipe_num > 3) { // error if more than 3 pipes
			perror("Only 1 to 3 pipes are supported.\n");
		}
		else if (pipe_num > 0) { // if 1 - 3 pipe exists 
			divide_buffer(buffer, divided_buffers, pipe_num); // divide buffer and store each command into the divided_buffers array
			
			// ERROR: Semaphore being stuck in waiting - why ?
			printf("semaphore waiting ...\n");
			sem_wait(sem_wq); // lock the semaphore
			printf("semaphore finished waiting !\n");

			// Add current program to waiting queue
			pid_t tid = pthread_self();
			// pid_t tid = 0; // instead ised dummy value, we might not need tid afterall
			int burst = 3;
			struct Program *program = create_program(tid, s, burst, pipe_num, args, divided_buffers);
			LIST_INSERT_HEAD(&waiting_queue, program, pointers);

			// Print all the elements in waiting queue to check if the program is added correctly
			struct Program *p;
			LIST_FOREACH(p, &waiting_queue, pointers) {
				printf("programs in waiting queue args[0]: %s\n", p->args[0]);
				printf("programs in waiting queue args[1]: %s\n", p->args[1]);
				printf("programs in waiting queue divided_buffers[0]: %s\n", p->divided_buffers[0]);
				printf("programs in waiting queue divided_buffers[1]: %s\n", p->divided_buffers[1]);
			}

			sem_post(sem_wq); // release the semaphore
			printf("semaphore released !\n");
		}
		else { // if no pipe
			get_argument_list(buffer, args); // divide user input and store each argument into an argument list
			
			// ERROR: Semaphore being stuck in waiting - why ?
			printf("semaphore waiting ...\n");
			sem_wait(sem_wq); // lock the semaphore
			printf("semaphore finished waiting !\n");

			// Add current program to waiting queue
			pid_t tid = pthread_self();
			// pid_t tid = 0; // instead used dummy value, we might not need tid afterall
			int burst = 3;
			struct Program *program = create_program(tid, s, burst, pipe_num, args, divided_buffers);
			LIST_INSERT_HEAD(&waiting_queue, program, pointers);
			printf("socket: %d\n", s);

			// Print all the elements in waiting queue to check if the program is added correctly
			struct Program *p;
			LIST_FOREACH(p, &waiting_queue, pointers) {
				printf("programs in waiting queue args[0]: %s\n", p->args[0]);
				printf("programs in waiting queue args[1]: %s\n", p->args[1]);
			}

			sem_post(sem_wq); // release the semaphore	
			printf("semaphore released !\n");		
		}
	}
	
	// if (strcmp(output, "EXIT") == 0) { // if exit
	// 	break;
	// }
	// }

	//close the sockets
	close(s);
}