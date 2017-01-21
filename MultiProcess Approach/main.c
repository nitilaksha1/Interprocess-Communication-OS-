#include "defines.h"

#define N 2
//#define DEBUG

/* main function. This function creates four child processes of which three are producers and one is a consumer.
* The producer processes get the COLOR code and the shared memory key as the parameters
* The consumer process gets only the shared memory key as its parameter
* This function initializes all the shared variables, mutex, and condition variables that will be passed between the processes.
*/
int main (int argc, char **argv) {

	key_t key = 4035;						///< Key to create the shared memory
	int shmid, k = 0, j = 0;			
	struct data *st;						///< Shared data structure that will be initialized
	pid_t childpid;
	char shmidstr[10];						///< String value of the shared memory key passed between processes

	//Create shared memory
	shmid = shmget (key, sizeof(struct data), 1023);

	if (shmid == -1) {
		#ifdef DEBUG
		printf("\r\nshmget failed: %d\r\n", errno);
		#endif
		exit (1);
	}
	
	//Initilize the shared memory
	st = (struct data *)shmat(shmid, (void *)NULL, 0);

	if (st == (void *) -1) {
		#ifdef DEBUG
		printf("\r\nshmat failed: %d\r\n", errno);
		#endif
		shmctl (shmid, IPC_RMID, NULL);
		exit(1);
	}

	//initializing the count variables and buffer indices
	st->count = 0;
	st->in = st->out = 0;

	//initializing the mutex attribute to make it shareable across processes
	pthread_mutexattr_init (&(st->mutexattr));
	pthread_mutexattr_setpshared (&st->mutexattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init (&(st->lock), &st->mutexattr);

	//initializing the condition attributes to make them shareable across processes
	pthread_condattr_init (&(st->condattr1));
	pthread_condattr_setpshared (&(st->condattr1), PTHREAD_PROCESS_SHARED);
    
    	pthread_condattr_init (&(st->condattr2));
    	pthread_condattr_setpshared (&(st->condattr2), PTHREAD_PROCESS_SHARED);
    
	//Initializing the condition variables with the attributes
	pthread_cond_init (&(st->spaceAvailable), &(st->condattr1));
	pthread_cond_init (&(st->itemsAvailable), &(st->condattr2));

	#ifdef DEBUG
	sprintf(shmidstr, "%d", key);
	#endif

	//Forking producer 1 which is assigned color RED
	childpid = fork();

	if (childpid == 0) {
		
		execl ("./producer", "./producer", "RED", shmidstr, NULL);
	}

	//Forking producer 2 which is assigned color BLACK
	childpid = fork();

	if (childpid == 0) {
		execl ("./producer", "./producer", "BLACK", shmidstr, NULL);
	}

	//Forking producer 3 which is assigned color WHITE
	childpid = fork();

	if (childpid == 0) {
		execl ("./producer", "./producer", "WHITE", shmidstr, NULL);
	}

	//Forking the consumer
	childpid = fork();

	if (childpid == 0) {
		execl ("./consumer", "./consumer", shmidstr, NULL);
	}

	//wait for all children to complete	
	wait (NULL);

	shmdt ((void *)st);

	//Deallocate shared memory
	shmctl (shmid, IPC_RMID, NULL);
	
	/*pthread_cond_destroy (&(st->spaceAvailable));
	pthread_cond_destroy (&(st->itemsAvailable));

	pthread_mutex_destroy (&(st->countmutex));
	pthread_mutex_destroy (&(st->buffermutex));*/

	return 0;
}
