/*
 * structure.h
 *
 *  Created on: Jan 31, 2013
 *      Author: sachit
 */
#pragma once
#ifndef STRUCTURE_H_
#define STRUCTURE_H_
#include <ucontext.h>

typedef struct gtthread_tcb gtthread_tcb;
typedef long gtthread_t;
typedef struct queue_node
{
	gtthread_tcb* thrcb;
	struct queue_node* next;
	struct queue_node* prev;
}queue_node;

typedef struct queue
{
	queue_node* front;
	queue_node* back;
	long nelements;
	queue_node* current;
	//add lock here
}queue_t;

typedef struct gtthread_tcb{
	ucontext_t* ctxt;
	gtthread_t thrid;
	int isblocked;
	int iscomplete;
	void* ret;
	queue_t join_queue;
	void* joinval;
}gtthread_tcb;

typedef struct gtthread_mutex_t{
	int lock; //Zero indicates unlocked. One indicates locked.
	long owner;


}gtthread_mutex_t;



#endif /* STRUCTURE_H_ */
