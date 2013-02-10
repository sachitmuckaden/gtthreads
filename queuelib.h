/*
 * queuelib.h
 *
 *  Created on: Jan 24, 2013
 *      Author: sachit
 */
#pragma once
#ifndef QUEUELIB_H_
#define QUEUELIB_H_


#include <ucontext.h>
#include "structure.h"




void queue_init(queue_t* q);
queue_node* queue_remove(queue_t* q,  long thrtid);
int queue_insert(queue_t* q, gtthread_tcb* qthr);
int queue_insert_normal(queue_t* q, gtthread_tcb* qthr);
long size_of_q(queue_t* q);
gtthread_tcb* queue_next(queue_t* q);
void queue_set_current(queue_t* q);
gtthread_tcb* queue_get(queue_t* q);
gtthread_tcb* queue_search(queue_t* q, gtthread_t thread);
void queue_unblock_all(queue_t* q, void* retval);
#endif /* QUEUELIB_H_ */
