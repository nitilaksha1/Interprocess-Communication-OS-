#include<stdlib.h>
#include<sys/time.h>
#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

//#define DEBUG
#define N 2
#define PRODITEMCOUNT 1000
#define CONSITEMCOUNT 3000

char 		buffer [2][50];		///< Shared buffer among the threads
int 		count = 0;		///< Shared count among the threads
int 		in = 0, out = 0;	///< Index variables for adding and removing items from buffer
pthread_cond_t	spaceAvailable = PTHREAD_COND_INITIALIZER, itemsAvailable = PTHREAD_COND_INITIALIZER;	///< condition variables for synchronization
pthread_mutex_t	lock = PTHREAD_MUTEX_INITIALIZER;	///< mutex for synchronization to shared variables

/*This function executes the producer code. All threads will call the this producer function*/
void * producer (void * arg) {

	struct timeval tval;		///< Structure to store the time of the day
	int fd;				///< File descriptor for log file updated by each thread

	#ifdef DEBUG
	printf("\r\nIn thread %d\r\n", *((int *)arg));
	#endif

	/* Producer loop */
	for (int i = 0; i < PRODITEMCOUNT; i++) {

		//Take mutex hold
		pthread_mutex_lock (&lock);
		
		//wait on condition spaceavailable if buffer is full
		while (count == N) {	
			 while (pthread_cond_wait( &spaceAvailable, &lock) != 0 ) ;
		}

		#ifdef DEBUG
		printf("\r\nIn thread %d: Getting time of the day\r\n", *((int *)arg));
		#endif

		//Create the buffer item string "COLOR timestamp" and write to log file
		gettimeofday(&tval, NULL);

		/* Create different files based on the producer number */		
		if (*((int *)arg) == 1) { 		

			sprintf(buffer[in], "RED %ld", tval.tv_usec);

			#ifdef DEBUG
			printf("\r\nIn thread %d: Buffer = %s\r\n", *((int *)arg), buffer[in]);
			#endif
	
			fd = open ("Producer_RED.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);

			#ifdef DEBUG
			printf("\r\nIn thread %d: File open descriptor = %d\r\n", *((int *)arg), fd);
			#endif

			if (fd == -1) {

				#ifdef DEBUG
				printf("\r\nExiting thread %d due open failure\r\n", *((int *)arg));
				#endif
	
				pthread_exit((void *)1);

			}
		} else if (*((int *)arg) == 2) {

			sprintf(buffer[in], "BLACK %ld", tval.tv_usec);

			#ifdef DEBUG
			printf("\r\nIn thread %d: Buffer = %s\r\n", *((int *)arg), buffer[in]);
			#endif
			fd = open ("Producer_BLACK.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);

			#ifdef DEBUG
			printf("\r\nIn thread %d: File open descriptor = %d\r\n", *((int *)arg), fd);
			#endif

			if (fd == -1) {
				#ifdef DEBUG
				printf("\r\nExiting thread %d due open failure\r\n", *((int *)arg));
				#endif
				pthread_exit((void *)1);

			}

		} else {
			sprintf(buffer[in], "WHITE %ld", tval.tv_usec);

			#ifdef DEBUG
			printf("\r\nIn thread %d: Buffer = %s\r\n", *((int *)arg), buffer[in]);
			#endif

			fd = open ("Producer_WHITE.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
			
			#ifdef DEBUG
			printf("\r\nIn thread %d: File open descriptor = %d\r\n", *((int *)arg), fd);
			#endif
		
			if (fd == -1) {
				#ifdef DEBUG	
				printf("\r\nExiting thread %d due open failure\r\n", *((int *)arg));
				#endif
				pthread_exit((void *)1);

			}

		}

		write (fd, buffer[in], strlen(buffer[in]));
		write (fd, "\r\n", 2);

		close (fd);
		
		#ifdef DEBUG
		printf("\r\nIn thread %d: File closed", *((int *)arg));
		#endif 

		count++;

		#ifdef DEBUG
		printf("\r\nIn thread %d: count = %d\r\n", *((int *)arg), count);
		#endif

		//Update the index
		in = (in + 1) % N;

		//release mutex hold
		pthread_mutex_unlock (&lock);

		//Signal the waiting consumer
		pthread_cond_signal (&itemsAvailable);
	}
	
	#ifdef DEBUG		
	printf("\r\nEnd of thread %d\r\n",*((int *)arg));
	#endif

	return (void *)0;
}

/* This function executes the consumer code. There is only one consumer thread that will access this function */
void * consumer (void * arg) {

		int fd;						///< File descriptor for the logfile updated by the consumer.

	#ifdef DEBUG
	printf("\r\nIn consumer thread :\r\n");
	#endif

	for (int i = 0; i < CONSITEMCOUNT; i++) {
		
		//Take mutex lock
		pthread_mutex_lock (&lock);
		
		//If buffer is empty , wait on condition itemsAvailable
		while (count == 0)
			while(pthread_cond_wait (&itemsAvailable, &lock) != 0);

		//Write the content from the buffer to Consumer.txt
		fd = open ("Consumer.txt", O_WRONLY | O_CREAT |O_APPEND, 0644);

		#ifdef DEBUG
		printf("\r\nIn consumer thread : File opened\r\n");
		#endif

		if (fd == -1) {
			#ifdef DEBUG
			printf("\r\nIn consumer thread : File open failed\r\n");
			#endif
			pthread_exit((void *)1);

		}
		write (fd, buffer[out], strlen(buffer[out]));
		write (fd, "\r\n", 2);
	
		#ifdef DEBUG
		printf("\r\nIn consumer thread :buffer written = %s\r\n", buffer[out]);
		#endif

		close (fd);

		count--;
		
		#ifdef DEBUG
		printf("\r\nIn consumer thread : count = %d\r\n", count);
		#endif

		//update the index
		out = (out + 1) % N;
		
		//Release mutex lock
		pthread_mutex_unlock (&lock);
		
		//Signal the Space available condition variable
		pthread_cond_signal (&spaceAvailable);

	}
	return (void *)0;
}

/* main function. This function creates four threads (3 producer and 1 consumer) and waits for the consumer thread to exit.*/
int main(void) {

	pthread_t 	prodthread1, prodthread2, prodthread3, consthread;		///< thread id of various threads
	int 		prodno1 = 1, prodno2 = 2, prodno3 = 3, err;				///< info for identifying the producer

	//Creating producer1 thread
	err = pthread_create (&prodthread1, NULL, producer,&prodno1);

	if (err != 0)
		exit(1);

	//Creating producer2 thread
	err = pthread_create (&prodthread2, NULL, producer,&prodno2);

	if (err != 0)
		exit(1);

	//Creating producer3 thread
	err = pthread_create (&prodthread3, NULL, producer,&prodno3);

	if (err != 0)
		exit(1);

	//Creating the consumer thread
	err = pthread_create (&consthread, NULL, consumer, NULL);

	if (err != 0)
		exit(1);

	//waiting for the consumer thread to exit
	err = pthread_join (consthread, NULL);

	if (err != 0)
		exit(2);

	return 0;
}
