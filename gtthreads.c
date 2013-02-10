/*
 * gtthreads.c
 *
 *  Created on: Jan 26, 2013
 *      Author: sachit
 */



#include "gtthreads.h"



void* schedule_next(int signum);
int initializesighandler();
void wrapper_function(void*(*start_routine(void)), void* params);

sigset_t threadprocmask;
void gtthread_init(long period)
{


	int rc = 0;
	if((rc=initializesighandler())<0)
	{
		printf("Signal handler error\n");
	}
	memset(&interval,0,sizeof(interval));

	interval.it_interval.tv_sec = 0;
	interval.it_interval.tv_usec = period;
	interval.it_value.tv_sec = 0;
	interval.it_value.tv_usec = period;
	timeslice = period;

	current_thr_id = 0;
	active_threads = 0;
	setmain = 0;
	queue_init(&readyqueue);
	queue_init(&deletequeue);
	main_thrcb = (gtthread_tcb*) malloc(sizeof(gtthread_tcb));

	//main_thrcb->ctxt = &main_ctxt;

	main_thrcb->thrid = 0;
	main_thrcb->isblocked = 0;
	main_thrcb->iscomplete = 0;
	queue_init(&main_thrcb->join_queue);
	main_thrcb->joinval = NULL;
	queue_insert(&readyqueue, main_thrcb);
	queue_set_current(&readyqueue);
	current_thrcb = main_thrcb;
	setitimer(ITIMER_VIRTUAL, &interval, NULL);

}
struct sigaction new_action;
int initializesighandler()
{

	sigemptyset(&threadprocmask);
	sigaddset(&threadprocmask, SIGVTALRM);

	memset(&new_action,0, sizeof(new_action));
	new_action.sa_handler = &schedule_next;
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;
	printf("Signal handler set up\n");


	if(sigaction (SIGVTALRM, &new_action, NULL)<0)
			return -1;
	return 1;
}

void wrapper_function(void*(*start_routine(void)), void* params)
{
	void* retval;
	printf("Timer: %ld\n",(long) interval.it_value.tv_usec );

	retval = start_routine();
	if(DEBUG)
	{
		printf("Wrapper received return value: %d\n", *(int*)retval);
	}
	gtthread_exit(retval);

}


int  gtthread_create(gtthread_t* thread,
                     void *(*start_routine)(void *),
                     void *arg)
{

	ucontext_t* ctxt;
	gtthread_tcb* tcb;
	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);
	ctxt = (ucontext_t*)malloc(sizeof(ucontext_t));
	if (getcontext(ctxt) == -1)
	{
		printf("Getcontext error\n");
		return -1;
	}
	char* stck =  (char*)malloc(STACK_SIZE* sizeof(char));
	ctxt->uc_stack.ss_sp =  stck;
	ctxt->uc_stack.ss_size = STACK_SIZE*sizeof(char);
	ctxt->uc_link = NULL;
	if(sigemptyset(&ctxt->uc_sigmask)<0)
	{
		return -1;
	}
	makecontext(ctxt, wrapper_function, 2, start_routine, arg);

	tcb = (gtthread_tcb*) malloc(sizeof(gtthread_tcb));
	tcb->ctxt = ctxt;

	tcb->thrid = ++current_thr_id;
	*thread = tcb->thrid;
	tcb->isblocked = 0;
	tcb->iscomplete = 0;
	tcb->joinval = NULL;
	queue_init(&tcb->join_queue);
	queue_insert(&readyqueue, tcb);

	printf("Create returning\n");
	return 1;
}



int  gtthread_join(gtthread_t thread, void **status)
{
	//First check if the thread is still running.
	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);
	gtthread_tcb* joinable = queue_search(&readyqueue, thread);
	if(joinable==NULL)
	{
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		return ESRCH;
	}

	queue_insert_normal(&joinable->join_queue,  current_thrcb);
	current_thrcb->isblocked =1;
	schedule_next(26);
	*status = current_thrcb->joinval;
	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
	return 0;
	//free(current_thr->joinval);
}

void gtthread_yield(void)
{
	schedule_next(SIGINT);
}

gtthread_t gtthread_self(void)
{
	return current_thrcb->thrid;
}

void gtthread_exit(void *retval)
{
	//Remember process shared resources should not be released. When implementing locks etc dp npt release locks on gtthread_exit
	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);
	current_thrcb->iscomplete = 1;
	queue_insert_normal(&deletequeue, (queue_remove(&readyqueue, current_thrcb->thrid))->thrcb);
	printf("Succesfully removed from queue\n");

	//SIGMASK ALL
	if(size_of_q(&(current_thrcb->join_queue))>0)
	{
		queue_unblock_all(&(current_thrcb->join_queue), retval);
	}

	//free(retval);
	if(size_of_q(&readyqueue)>0)
	{
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		schedule_next(SIGINT);
	}
	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
}

int  gtthread_cancel(gtthread_t thread)
{
	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);
	int* retval = (void*) malloc(sizeof(int));
	*retval = GTTHREAD_CANCELLED;
	gtthread_tcb* canceltcb = queue_search(&readyqueue, thread);
	if(canceltcb==NULL)
	{
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		return -1;
	}

	canceltcb->iscomplete = 1;
	queue_insert_normal(&deletequeue, (queue_remove(&readyqueue, thread))->thrcb);
	//queue_remove(&readyqueue, thread);
	printf("Succesfully removed from queue\n");


	//SIGMASK ALL
	if(size_of_q(&(canceltcb->join_queue))>0)
	{
		queue_unblock_all(&(canceltcb->join_queue), (void*)retval);
	}

	//free(retval);

	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
	return 1;
	//gtthread_exit(retval);
}





void* schedule_next(int signum)
{

	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);
	if((size_of_q(&readyqueue))>1)//if false this means only the main thread is in the queue
	{
		printf("Scheduling next thread-----------------\n");
		gtthread_tcb* tmpthr;
		tmpthr = current_thrcb;
		current_thrcb = queue_next(&readyqueue);
		while(current_thrcb->isblocked)
		{
			printf("Current thread blocked: %ld", (long)current_thrcb->thrid);
			current_thrcb = queue_next(&readyqueue);

		}
		printf("Swapping %ld for %ld\n", tmpthr->thrid, current_thrcb->thrid);
		interval.it_value.tv_usec = timeslice;
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		swapcontext(tmpthr->ctxt, current_thrcb->ctxt);
	}
	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
	return (void*) 0;
}
