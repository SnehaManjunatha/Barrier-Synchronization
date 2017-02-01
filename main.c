/*
* syscall(352) is barrier_wait system call
* syscall(351) is barrier_init system call
* syscall(353) is barrier_destroy system call
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <fcntl.h> // open function
#include <errno.h>

#define THREAD_SET_1   5
#define THREAD_SET_2   20
#define SYNC_ROUND     100

pid_t array_pid[2];
extern int errno;
/* structure to pass information to the thread*/

struct arg
{
	unsigned int n;
	unsigned int bar_id;				//barrier id
};

void *test_func(void *m_arg)
{
	int res;
	int a=0;
	struct arg *th_args ;
	
	for( a= 0; a < SYNC_ROUND; a++)
	{
		if (array_pid[0] == getpid())
		printf("	%d round of synchronization : Thread entering : pid is %ld my thread id is %ld\n",a+1,syscall(SYS_getpid),syscall(SYS_gettid));
		if (array_pid[1] == getpid())
		printf("		%d round of synchronization : Thread entering : pid is %ld my thread id is %ld\n",a+1,syscall(SYS_getpid),syscall(SYS_gettid));
		th_args = (struct arg *)m_arg;
		res = syscall(352, th_args->bar_id);			//barrier wait sys call
		
		if(res<0)
		{
			printf("barrier_wait operation failed\n");
			perror("error :");

		}
		if (array_pid[0] == getpid())
		printf("	%d round of synchronization : Thread exiting : pid is %ld my thread id is %ld\n",a+1,syscall(SYS_getpid),syscall(SYS_gettid));
		if (array_pid[1] == getpid())
		printf("		%d round of synchronization : Thread exiting : pid is %ld my thread id is %ld\n",a+1,syscall(SYS_getpid),syscall(SYS_gettid));
		sleep(1);
	}	
	pthread_exit(0);
}


void *test_func2(void *m_arg)
{
	int res;
	int a=0;
	struct arg *th_args;

	
	for( a= 0; a<SYNC_ROUND; a++)
	{
		if (array_pid[0] == getpid())
		printf("	%d round of synchronization : Thread entering : pid is %ld my thread id is %ld\n",a+1,syscall(SYS_getpid),syscall(SYS_gettid));
		if (array_pid[1] == getpid())
		printf("		%d round of synchronization : Thread entering : pid is %ld my thread id is %ld\n",a+1,syscall(SYS_getpid),syscall(SYS_gettid));
		th_args = (struct arg *)m_arg;
		res = syscall(352, th_args->bar_id);			//barrier wait sys call
		if(res<0)
		{
			printf("barrier_wait operation failed\n");
			perror("error :");
		}
		
		if (array_pid[0] == getpid())
		printf("	%d round of synchronization : Thread exiting : pid is %ld my thread id is %ld\n",a+1,syscall(SYS_getpid),syscall(SYS_gettid));
		if (array_pid[1] == getpid())
		printf("		%d round of synchronization : Thread exiting : pid is %ld my thread id is %ld\n",a+1,syscall(SYS_getpid),syscall(SYS_gettid));
	sleep(1);
	}	
	pthread_exit(0);
}


void Childprocess()
{
	unsigned int buf, bar_id1, bar_id2;
	int set;
	int i,j;
   	pid_t my_pid;
	pthread_t tid[THREAD_SET_1];
	pthread_t tid2[THREAD_SET_2];
	
	my_pid = getpid();    
	printf("\nChild: my pid is: %d\n", my_pid);

	struct arg *parg= (struct arg *) malloc (sizeof(struct arg));
	unsigned int *u_bar_id1= (unsigned int *) malloc (sizeof(unsigned int));
 	unsigned int *u_bar_id2= (unsigned int *) malloc (sizeof(unsigned int));

	buf = THREAD_SET_1;	
	if ((set = syscall(351,buf,u_bar_id1))==-1)      	//barrier init sys call
		printf("barrier_init failed\n");
	bar_id1 = *u_bar_id1;
	for(i=0; i<THREAD_SET_1; i++)
	{
		parg->n = i;
		parg->bar_id = bar_id1;
		pthread_create(&tid[i],NULL,test_func,(void*)parg);
	}
		
	buf = THREAD_SET_2;
	if ((set =syscall(351,buf, u_bar_id2))==-1)    
		printf("barrier_init failed\n");		//barrier init sys call
	bar_id2 = *u_bar_id2;
	for(i=0; i<THREAD_SET_2; i++)
	{
		parg->n = i;
		parg->bar_id = bar_id2;
		pthread_create(&tid2[i],NULL,test_func2,(void*)parg);
	}
	
	for(j=0;j<THREAD_SET_1;j++)
	{
		pthread_join(tid[j],NULL);
	}

	for(j=0;j<THREAD_SET_2;j++)
	{
		pthread_join(tid2[j],NULL);
	}
sleep(1);
	if ((set =syscall(353,bar_id1))==-1)    		//barrier destroy sys call
		printf("USER_SPACE : barrier id= %d not deleted\n",bar_id1);
sleep(1);
	if ((set =syscall(353,bar_id2))==-1)    		//barrier destroy sys call
		printf("USER_SPACE : barrier id= %d not deleted\n",bar_id2 );
	free(parg);
        free(u_bar_id2);
      	free(u_bar_id1);

   	_exit(1);
}

int main( int argc, char *argv[], char *env[] )
{
	pid_t ppid = getpid();
        printf("Parent: my pid is: %d\n", ppid);

	pid_t child_pid1 , child_pid2;
	child_pid1 = fork();		
	if(child_pid1  < 0 )
	{
      		perror("fork failure");
      		exit(1);
   	}
   	else if (child_pid1 == 0)	//created the first child
   	{ 
		array_pid[0] = getpid();
		Childprocess();
   	}
	else if (child_pid1 > 0) //Parent
	{
		child_pid2 = fork();
		if(child_pid2  < 0 )
		{	
      			perror("fork failure");
      			exit(1);
   		}
		else if(child_pid2 == 0)	//created the second child
   		{ 
			array_pid[1] = getpid();
			Childprocess();
   		}
		else if( child_pid2 > 0) //parent
		{
			wait(NULL);
		}
    		wait(NULL);
	}
  	printf("Parent: Exited child process\n");
   	return 0;
}
