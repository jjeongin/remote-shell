//from https://www.stechies.com/round-robin-scheduling/
#include<stdio.h>
#include<stdlib.h>
int processes[100][3], NP, quantum, ProcessScheduler[1000],WaitingTime[100];
unsigned int time = 0;
typedef struct el
{
	unsigned int p;
	struct el * next;
}Q;
Q * queue = NULL;

//Function to get input from user (e.g. number of process, Quantum Time etc )

void getSystem()
{
	int i;
	printf("\nNumber of processes: ");
	scanf("%d", &NP);
	
	printf("\nThe Quantum: ");
	scanf("%d", &quantum);
	
	for(i=0; i<NP; i++ )
	{
		printf("\n Arrival Time of process%d: ", i);
		scanf("%d", &processes[i][0]);
		printf("\n Burst time for process%d: ", i);
		scanf("%d", &processes[i][1]);
		processes[i][2] = processes[i][1];
		printf("\n-----------");
	}
}

//Printing the output for which process is running and how much remaining for the process
void printSystem()
{
	int i;
	printf("\n\n******************************************************************");
	printf("\nQuantum Time: %d",quantum);
	printf("\nProcess:  Arrival-Time  Burst-Time Remaining-Time");
	for(i=0; i<NP; i++)
	{
		printf("\nProcess%d:  \t%d  \t\t%d  \t\t%d", i, processes[i][0], processes[i][1], processes[i][2]);
		printf("\n______________________________________________________");
	}
	printf("\nProcess in the Queue: ");
	Q *n;
	for(n=queue; n!=NULL; n=n->next)
	{
		printf("Process%d ",n->p);
	}
}

//Function to get remaining time for the process

unsigned int executionRemained()
{
	int i;
	unsigned int x = 0;
	for(i=0; i<NP; i++)
	{
		if(processes[i][2] > 0)
		{
			x = 1;
		}
	}
	return x;
}

//Function to add process in the Queue
void addToQueue(int i)
{
	Q *n, *n1;
	n = (Q *)malloc(sizeof(Q));
	n->next = NULL;
	n->p = i;
	if(queue == NULL)
	{
		
		queue = n;
	}
	else
	{
		for(n1 = queue ; n1->next!=NULL; n1=n1->next);
		n1 -> next = n;
	}
}

//Function Adding the arrived process into the queue
void addArrivedProcessesToQueue()
{
	int i;
	for(i=0; i<NP; i++)
	{
		if(processes[i][0] == time)
		{
			addToQueue(i);
		}
	}
}

//Function returns the next process in the queue
unsigned int getNextProcess()
{
	Q *n;
	int x;
	if(queue == NULL)
	{
		return -1;
	}
	else
	{
		x = queue -> p;
		n = queue;
		queue = queue -> next;
		free(n);
		return x;
	}
}

//function to get the schedule of the process

void schedule()
{
	unsigned int np, toRun, q, i;
	q = 0;
	addArrivedProcessesToQueue();
	while(executionRemained())
	{
		np = getNextProcess();
		if(np == -1)
		{
			/*
			here if there is no process in waiting queue
			which mean the process get IDLe state.
			here in this program we put -1 in ProcessScheduler[time]
			which mean that the processor get IDLE in this time.
			
			*/
			ProcessScheduler[time] = -1;
			time++;
			addArrivedProcessesToQueue();
		}
		else
		{
			q = quantum;
			if(processes[np][2] < q)
			{
				q = processes[np][2];
			}
			for(i = q; i>0; i--)
			{
				ProcessScheduler[time]=np;
				time++;
				processes[np][2]--;
				addArrivedProcessesToQueue();
			}
			if(processes[np][2] > 0)
			{
				addToQueue(np);
			}
		}
		
		
		printSystem();
		int x;
		
	}
}

//Function to print the scheduling of the process during the life time
//This function also give the average waiting time for all the process
void printScheduling()
{
	int i;
	printf("\n\nScheduling Progress Throughout the program : \n");
	for(i=0; i<time; i++)
	{
		printf("\n[%d-%d] (Process%d) \n",i,i+1 ,ProcessScheduler[i]);
		printf("\n______________________________");
	}
	printf("\n\nWaiting Time Per Process : \n");
	for(i=0; i<NP; i++)
	{
		printf("\nP%d: %d", i, WaitingTime[i]);
	}
	//counting Average Waiting Time...
	float AverageWaitingTime = 0.0;
	for(i=0; i<NP; i++)
	{
		AverageWaitingTime = AverageWaitingTime+WaitingTime[i];
	}
	AverageWaitingTime = AverageWaitingTime/NP;
	printf("\n\nAverage Waiting Time: %f", AverageWaitingTime);
	
}

//Function to get the waiting time for all the process individually

void WatingTime()
{
	int i;
	unsigned int releaseTime, t;
	for(i=0; i<NP; i++)
	{
		
		for(t=time-1; ProcessScheduler[t]!= i; t--);
		releaseTime = t+1;
		WaitingTime[i] = releaseTime - processes[i][0] - processes[i][1];
	}
}

//Main function of the program
main()
{
	getSystem();
	printSystem();
	schedule();
	WatingTime();
	printScheduling();
}