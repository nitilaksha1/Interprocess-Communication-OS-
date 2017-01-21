#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

#define N 2

/* Data that is shared among the producer and consumer processes*/
struct data {
	pthread_cond_t spaceAvailable;
	pthread_cond_t itemsAvailable;
	int count, in, out;
	pthread_mutex_t lock;
	pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr1;
    pthread_condattr_t condattr2;
	char buffer[N][50];
};


