/*
 * gtthreads.h

 *
 *  Created on: Jan 26, 2013
 *      Author: sachit
 */

#pragma once
#ifndef GTTHREADS_H_
#define GTTHREADS_H_


#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include "queuelib.h"
#include "values.h"
#include "structure.h"
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define GTTHREAD_CANCELLED 15

typedef struct gtthread_mutex_t{

}gtthread_mutex_t;

//global variables

struct itimerval interval;
gtthread_tcb* current_thrcb;
gtthread_tcb* main_thrcb;
long current_thr_id;
int active_threads;
int setmain;
long timeslice;
queue_t readyqueue;
queue_t deletequeue;
ucontext_t main_ctxt;



void gtthread_init(long period);
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);

int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);


int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);

#endif /* GTTHREADS_H_ */
