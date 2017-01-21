#include "defines.h"

#define IPC_ALLOC 0100000
#define NOOFITEMS 1000

//#define DEBUG

/*This function runs the producer code. Each producer will add 1000 items to the buffer and to their respective log files*/
int main (int argc, char **argv) {

	int shmid, fd;
    	key_t key;
	struct data *st = NULL;
	struct timeval tval;
	int sharedflag;

	//Extracting the shared memory key passed by the parent process
	key = atoi (argv[2]);

	shmid = shmget (key, 0, IPC_ALLOC);

	if (shmid == -1) {
		#ifdef DEBUG
        printf("\r\nshmget in producer failed: %d\r\n", errno);
		#endif
		exit(1);
	}

	#ifdef DEBUG
	printf("\r\nShared memory fetched in producer %s\r\n", argv[1]);
	#endif

	//attaching to the memory segment
	st = (struct data *)shmat (shmid, NULL, 1023);

	if (st == (void *) -1) {
		#ifdef DEBUG
		printf("\r\nshmat in producer %s failed!! Code = %d", argv[1], errno);	
		#endif
		exit(1);
	}

	#ifdef DEBUG
	printf("\r\nShared memory attached in producer %s\r\n", argv[1]);
	#endif

	for (int i = 0; i < NOOFITEMS; i++) {
		
		//Taking the mutex hold before modifying buffer
		pthread_mutex_lock (&(st->lock));

		#ifdef DEBUG
		printf("\r\nIn producer %s\r\n", argv[1]);
		#endif

		//Check to see if space is available in buffer to put item
		while (st->count == N) {
			while (pthread_cond_wait(&(st->spaceAvailable), &(st->lock)) != 0);
		}
		
		gettimeofday(&tval, NULL);
		
		//Create Item string and put in the buffer
		sprintf(st->buffer[st->in], "%s %ld", argv[1], tval.tv_usec);

		//Creating different log files based on the COLOR argument passed		
		if (strcmp(argv[1], "BLACK") == 0) {
			fd = open ("Producer_BLACK.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
		} else if (strcmp(argv[1], "RED") == 0) {
			fd = open ("Producer_RED.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
		} else {
			fd = open ("Producer_WHITE.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
		}

		if (fd == -1) {
			#ifdef DEBUG
			printf ("\r\nFile open failed in producer\r\n");
			#endif
			exit(1);
		}
	
		//Writing the buffer contents to the logfile
		write (fd, st->buffer[st->in], strlen(st->buffer[st->in]));
		write (fd, "\r\n", 2);
		
		#ifdef DEBUG
		printf("\r\nProducer %s wrote buffer %s\r\n", argv[1], st->buffer[st->in]);
		#endif

		st->in = ((st->in) + 1) % N;
		
		close (fd);
		
		st->count++;	

		#ifdef DEBUG
		printf("\r\ncount = %d\r\n", st->count);	
		#endif

		pthread_cond_signal (&(st->itemsAvailable));
		pthread_mutex_unlock(&(st->lock));

	}
	
	if (shmdt ((void *)st) == -1) {
		#ifdef DEBUG
		printf("\r\nshmdt in producer failed!! Code = %d", errno);
		#endif 
		exit(1);	
	}

	return 0;
}
