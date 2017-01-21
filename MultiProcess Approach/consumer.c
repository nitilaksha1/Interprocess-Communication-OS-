#include "defines.h"

#define IPC_ALLOC 0100000
#define N 2
#define NOOFITEMS 3000


/* This function executes the consumer code. The consumer code runs till removes 3000 items*/
int main (int argc, char **argv) {
	
	int shmid, fd, pshared;
    key_t key;
	struct data *st = NULL;
	struct timeval tval;

	key  = atoi (argv[1]);

	shmid = shmget (key , 0, IPC_ALLOC);

	if (shmid == -1) {
		#ifdef DEBUG
		printf("\r\nshmget failed in consumer\r\n");
		#endif
		exit(1);
	}

	#ifdef DEBUG
	printf("\r\nShared memory fetched in consumer\r\n");
	#endif

	//attaching to the memory segment
	st = (struct data *)shmat (shmid, (void *)NULL, 1023);

	#ifdef DEBUG
	printf("\r\nShared memory attached in Consumer\r\n");
	#endif

	if (st == (void *) -1) {
		#ifdef DEBUG
		printf("\r\nshmat in consumer failed!! Code = %d", errno);
		#endif
		exit(1);
	}

	pthread_mutexattr_getpshared(&(st->mutexattr), &pshared);

	#ifdef DEBUG
	if (pshared == PTHREAD_PROCESS_SHARED)
		printf("\r\nMutex is in shared mode in consumer\r\n");
	#endif

	for (int i = 0; i < NOOFITEMS; i++) {

		pthread_mutex_lock (&(st->lock));
		
		#ifdef DEBUG
		printf("\r\nIn consumer\r\n");
		#endif

		//Check to see if space is available in buffer to put item
		while (st->count == 0) {
			while (pthread_cond_wait(&(st->itemsAvailable), (&(st->lock))) != 0);
		}

		#ifdef DEBUG
		printf("\r\nConsumer wait completed\r\n");
		#endif

		fd = open ("Consumer.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
		
		if (fd == -1) {
			#ifdef DEBUG
			printf ("\r\nFile open failed in consumer\r\n");
			#endif
			exit(1);
		}

		write (fd, st->buffer[st->out], strlen(st->buffer[st->out]));		
		write (fd, "\r\n", 2);	
		close (fd);

		#ifdef DEBUG
		printf("\r\nCosnumer wrote buffer %s\r\n", st->buffer[st->out]);
		#endif

		st->out = (st->out + 1) % N;
		st->count--;	

		#ifdef DEBUG
		printf("\r\ncount = %d\r\n", st->count);
		#endif

		pthread_cond_signal (&(st->spaceAvailable));
		
		pthread_mutex_unlock (&(st->lock));		

	}
	
	if (shmdt ((void *)st) == -1) {
		#ifdef DEBUG
		printf("\r\nshmdt in consumer failed!! Code = %d", errno);
		#endif
		exit(1);	
	}

	return 0;
}
