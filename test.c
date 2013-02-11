/*
 * test.c
 *
 *  Created on: Jan 31, 2013
 *      Author: sachit
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gtthreads.h"


void* routine1(void* params);
void* routine2(void* params);

gtthread_mutex_t mymutex;
void main(int argc, char** argv)
{
	gtthread_t thread1, thread2;
	int i;
	double j;
	long period = 30;
	printf("This is the main function. \n");
	printf("Initializing the gtthreads with period %ld\n", period);
	gtthread_init(period);
	gtthread_mutex_init(&mymutex);
	printf("gtthreads initialized.\n");
	printf("Creating the first thread\n");
	gtthread_create(&thread1, &routine1, NULL);
	printf("Thread id: %ld\n", (long)gtthread_self());
	printf("Back in the main thread. Creating the next thread\n");
	gtthread_create(&thread2, &routine2, (void*)&thread1);
	printf("Thread id: %ld\n", (long)gtthread_self());

	void* returnvalue1, *returnvalue2;
	gtthread_join(thread2, &returnvalue1 );
	gtthread_join(thread1, &returnvalue2);
	printf("Back in the main thread\nDoing complex calculation\n");
	printf("Received from thread 2: %d\nReceived from thread 1: %d\n", *(int*)returnvalue1, *(int*)returnvalue2);
	//while(1){}
	printf("Main thread completed calculation\n Exiting\n");

}

void* routine1(void* params)
{
	printf("Entered routine 1\nAttempting calculation\n");
	int i;
	double j;
	gtthread_mutex_lock(&mymutex);
	for(i=0;i<100000;i++)
	{
		j += 0.957636*0.6537728;
		printf("%f6\n", j);
	}
	gtthread_mutex_unlock(&mymutex);
	//sleep(10);

	printf("Ending routine 1.\n");
	fflush(stdout);
	int* b = (int*) malloc(sizeof(int));
	//while(1){}
	*b = 10;
	//printf("Should never reach here*****************\n");
	return (void*)b;
}

void* routine2(void* params)
{
	printf("Entered routine 2\nAttempting calculation\n");
	int i;
	double j;
	gtthread_mutex_lock(&mymutex);
	for(i=0;i<1000;i++)
	{
		j += 0.9562637*0.657726;
		//printf("%f", j);
	}
	gtthread_mutex_unlock(&mymutex);
	//sleep(10);
	//printf("Cancelling routine 1.\n");
	//gtthread_cancel(*(int*)params);
	printf("Ending routine 2.\n");
	fflush(stdout);
	//while(1){}
	int* b = (int*) malloc(sizeof(int));
	*b = 20;

	gtthread_exit( (void*)b);
}
