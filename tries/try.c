// from project description
	// In case the server is handling a request that solely includes a command, then the server should schedule it for one round of execution.
	// In case the server is handling a request with the value n, 
			//this means it will simulate the execution time of a process based on the "n" value. 
	//If another command or request arrived, the scheduler module should stop the execution of the current task if the scheduling algorithm selects this option based on your set conditions, save its id and the remaining value of "n", and execute the new task.
	// Once it is done, the server should loop over the list of waiting tasks and schedule the one that fits the best based on your scheduling algorithm and remaining amount of execution time.


// steps:
// declare the quantum q, and a time vairble t and the number of processes n
// keep track of completed processes
// create a loop that incremints n until the arrival time of the first process
// 		if process is a command, exceute all directly
// 		if handling a request, exec request until q is reached
// move this process to the waiting queue

// if more than one processes is the queue, then we itterate over them and execute commands(fcfs bases) and then check for the shortest burst time 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void sheduler(int n,/*list of threads*/){
	// reorganize the thread 
	check_for_SJR()

	int Q = 3 //quantum
	int t = 0 //time

	while(/*list of threads is not empty*/)
	{
		//run code based on rr

		bool flag = true;
		for(int i = 0; i < n; i++){
			if(temp_burst[i] != 0){
				flag = false;
				break;
			}
		}
		if(flag)
			break;

		for(int i = 0; (i < n) && (queue[i] != 0); i++){
			int ctr = 0;
			while((ctr < tq) && (temp_burst[queue[0]-1] > 0)){
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
	check_for_SJR()

}

void check_for_SJR(int queue[]){ //return list

	//sort the array in ascending order

		//from https://www.sanfoundry.com/c-program-sort-array-ascending-order/
		// for (i = 0; i < n; ++i) 
  //       {
 
  //           for (j = i + 1; j < n; ++j)
  //           {
 
  //               if (queue[i] > queue[j]) 
  //               {
  //                   a =  queue[i];
  //                   queue[i] = queue[j];
  //                   queue[j] = a;
 
  //               }
 
  //           }
  //       }
	return 0;
}



int main(void)
{


}
