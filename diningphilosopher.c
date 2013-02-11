/*
 * diningphilosopher.c
 *
 *  Created on: Feb 11, 2013
 *      Author: sachit
 */


#include <stdio.h>
#include <string.h>
#include <gtthread.h>
#include <time.h>
#define N 5
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int forks[N];
gtthread_mutex_t locks[N];
gtthread_t philosophers[N];
int* taskids[N];

void *worker(void* args);
void eat(int, int, int);
void think(int, int, int);

void main(int argc, char** argv)
{
	int i;
	gtthread_init(1000);
	for(i=0;i<N;i++)
	{
		forks[i] = 0;
		gtthread_mutex_init(&locks[i]);
	}
	for(i=0;i<N;i++)
	{
		taskids[i] = (int *) malloc(sizeof(int));
		*taskids[i] = i+1;
		gtthread_create(&philosophers[i], &worker, (void*) taskids[i]);
	}
	for(i=0;i<N;i++)
	{
		gtthread_join(philosophers[i], NULL);
	}

}

void *worker(void* params)
{
	int id = *(int*)params;
	int	higher, lower;
	higher = max((id-1),( id%5));
	lower = min((id-1), (id%5));
	while(1)
	{
		eat(lower,higher, id);
		think(lower, higher, id);
	}
	return (void*) 0;
}

void eat(int lower, int higher, int id)
{
	gtthread_mutex_lock(&locks[lower]);
	printf("Philosopher #%d tries to acquire left chopstick\n", id);
	while(forks[lower]==1)
	{
		gtthread_mutex_unlock(&locks[lower]);
		gtthread_yield();
		gtthread_mutex_lock(&locks[lower]);
	}
	forks[lower] =1;
	printf("Philosopher #%d acquired left chopstick\n", id);
	gtthread_mutex_unlock(&locks[lower]);


	gtthread_mutex_lock(&locks[higher]);
	printf("Philosopher #%d tries to acquire right chopstick\n", id);
	while(forks[higher]==1)
	{
		printf("Philosopher #%d is hungry\n",id);
		gtthread_mutex_unlock(&locks[higher]);
		gtthread_yield();
		gtthread_mutex_lock(&locks[higher]);
	}
	forks[higher] =1;
	printf("Philosopher #%d acquired right chopstick\n", id);
	gtthread_mutex_unlock(&locks[higher]);

	printf("Philosopher #%d is eating\n", id);
	int k;
	int num = (rand()%10 + 1)*100000;
	for(k=0;k<num;k++);

	gtthread_mutex_lock(&locks[higher]);
	printf("Philosopher #%d releasing right chopstick\n", id);
	forks[higher]=0;
	gtthread_mutex_unlock(&locks[higher]);


	gtthread_mutex_lock(&locks[lower]);
	printf("Philosopher #%d releasing left chopstick\n", id);
	forks[lower]=0;
	gtthread_mutex_unlock(&locks[lower]);

}

void think(int lower, int higher, int id)
{
	printf("Philosopher #%d is thinking\n", id);
	int k;
	int num = (rand()%10 + 1)*100000;
	for(k=0;k<num;k++);

	//random waiting time
}
