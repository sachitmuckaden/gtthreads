/*
 * queuelib.c
 *
 *  Created on: Jan 24, 2013
 *      Author: sachit
 */
#include "queuelib.h"
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1

void queue_init(queue_t* q)
{
	q->nelements = 0;
	q->front = NULL;
	q->back = NULL;
}

queue_node* queue_remove(queue_t* q,  long thread)
{
	if(q->nelements == 0)
	{
		if(DEBUG)
			printf("Attempted to remove node from empty queue");
		return NULL;
	}

	queue_node* qnode = q->front;
	while(qnode->thrcb->thrid!=thread)
	{
		qnode = qnode->next;
	}
	if(qnode->thrcb->thrid == q->front->thrcb->thrid)
	{
		q->front = q->front->next;
	}
	else if(qnode->thrcb->thrid == q->back->thrcb->thrid)
	{
		q->back = q->back->prev;
	}
	qnode->prev->next = qnode->next;
	qnode->next->prev = qnode->prev;
	q->nelements--;


	return qnode;
}

int queue_insert(queue_t* q, gtthread_tcb* qthrcb)
{
	queue_node* qnode = (queue_node*) malloc(sizeof(queue_node));
	qnode->thrcb = qthrcb;
	if(q->nelements==0)
	{
		q->front = q->back = qnode;
		qnode->prev = NULL;
	}
	else
	{
		q->back->next=qnode;
		qnode->prev = q->back;
		q->back = qnode;

	}
	qnode->next = q->front;
	q->nelements++;
	return 1;
}

gtthread_tcb* queue_next(queue_t* q)
{
	q->current = q->current->next;
	return q->current->thrcb;
}

long size_of_q(queue_t* q)
{
	return q->nelements;
}

void queue_set_current(queue_t* q)
{
	q->current= q->back;
}

gtthread_tcb* queue_get(queue_t* q)
{
	if(q->nelements == 0)
	{
		if(DEBUG)
			printf("Attempted to remove node from empty queue");
		return NULL;
	}
	queue_node* tmpnode;
	tmpnode = q->front;
	q->front = q->front->next;
	q->nelements--;
	return tmpnode->thrcb;
}

gtthread_tcb* queue_search(queue_t* q, gtthread_t thread)
{
	if(q->nelements==0)
	{
		return NULL;
	}
	queue_node* current_node;
	current_node = q->front;
	while(current_node!=NULL)
	{
		if(current_node->thrcb->thrid==thread)
			return current_node->thrcb;

		current_node = current_node->next;
	}
	return NULL;
}

void queue_unblock_all(queue_t* q, void* retval)
{
	queue_node* current_node;
	if(q->nelements == 0)
		return;
	current_node = q->front;
	while(current_node!=NULL)
	{
		current_node->thrcb->isblocked = 0;
		current_node->thrcb->joinval = retval;
		printf("Unblocked: %ld\n", (long)current_node->thrcb->thrid);
		current_node = current_node->next;
		//*current_node->thr->joinval = *retval;
	}
}

int queue_insert_normal(queue_t* q, gtthread_tcb* qthrcb)
{
	queue_node* qnode = (queue_node*) malloc(sizeof(queue_node));
	qnode->thrcb = qthrcb;
	if(q->nelements==0)
	{
		q->front = q->back = qnode;
		qnode->prev = NULL;
	}
	else
	{
		q->back->next=qnode;
		qnode->prev = q->back;
		q->back = qnode;

	}
	qnode->next = NULL;
	q->nelements++;
	return 1;
}

