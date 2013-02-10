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

void main(int argc, char** argv)
{
	gtthread_t thread1, thread2;
	int i;
	double j;
	long period = 250;
	printf("This is the main function. \n");
	printf("Initializing the gtthreads with period %ld\n", period);
	gtthread_init(period);
	printf("gtthreads initialized.\n");
	printf("Creating the first thread\n");
	gtthread_create(&thread1, &routine1, NULL);
	printf("Thread id: %ld", (long)gtthread_self());

	gtthread_create(&thread2, &routine2, NULL);
	printf("Thread id: %ld", (long)gtthread_self());
	printf("Back in the main thread. Creating the next thread\n");
	void* returnvalue;
	gtthread_join(thread2, &returnvalue );
	printf("Back in the main thread\nDoing complex calculation\n");
	printf("Received from thread 2: %d\n", *(int*)returnvalue);
	printf("Main thread completed calculation\n Exiting\n");

}

void* routine1(void* params)
{
	printf("Entered routine 1\n. Attempting calculation\n");
	int i;
	double j;
	for(i=0;i<100000;i++)
	{
		j += 0.957636*0.6537728;
		//printf("%f6\n", j);
	}
	sleep(10);

	printf("Ending routine 1.\n");
	int* b = (int*) malloc(sizeof(int));
	*b = 10;
	return (void*)b;
}

void* routine2(void* params)
{
	printf("Entered routine 2\n. Attempting calculation\n");
	int i;
	double j;
	for(i=0;i<1000;i++)
	{
		j += 0.9562637*0.657726;
		//printf("%f", j);
	}
	sleep(10);
	printf("Ending routine 2.\n");
	int* b = (int*) malloc(sizeof(int));
	*b = 20;
	return (void*)b;
}
