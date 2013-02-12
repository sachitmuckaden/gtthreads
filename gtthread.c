/*
 * gtthreads.c
 *
 *  Created on: Jan 26, 2013
 *      Author: sachit
 */



#include "gtthread.h"



void* schedule_next(int signum);
int initializesighandler();
void wrapper_function(void*(*start_routine(void*)), void* params);

sigset_t threadprocmask;
void gtthread_init(long period)
{


	int rc = 0;
	if((rc=initializesighandler())<0)
	{
		if(DEBUG)printf("Signal handler error\n");
	}
	memset(&interval,0,sizeof(interval));

	interval.it_interval.tv_sec = 0;
	interval.it_interval.tv_usec = 0;
	interval.it_value.tv_sec = 0;
	interval.it_value.tv_usec = period;
	timeslice = period;

	current_thr_id = 0;
	active_threads = 0;
	setmain = 0;
	queue_init(&readyqueue);
	queue_init(&deletequeue);
	main_thrcb = (gtthread_tcb*) malloc(sizeof(gtthread_tcb));



	main_thrcb->thrid = 0;
	main_thrcb->isblocked = 0;
	main_thrcb->iscomplete = 0;
	main_thrcb->ctxt =&main_ctxt;
	queue_init(&main_thrcb->join_queue);
	main_thrcb->joinval = NULL;
	queue_insert(&readyqueue, main_thrcb);
	queue_set_current(&readyqueue);
	current_thrcb = main_thrcb;
	initialized = 1;
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
	if(DEBUG)printf("Signal handler set up\n");


	if(sigaction (SIGVTALRM, &new_action, NULL)<0)
			return -1;
	return 1;
}

void wrapper_function(void*(*start_routine(void*)), void* params)
{
	void* retval;
	//if(DEBUG)printf("Timer: %ld\n",(long) interval.it_value.tv_usec );

	retval = start_routine(params);
	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);

	if(DEBUG)
	{
		//if(DEBUG)printf("Wrapper received return value: %d\n", *(int*)retval);
	}
	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
	gtthread_exit(retval);

}


int  gtthread_create(gtthread_t* thread,
                     void *(*start_routine)(void *),
                     void *arg)
{

	ucontext_t* ctxt;
	gtthread_tcb* tcb;
	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);

	//Check to see if gtthread_init has been run. If not exit
	if(!initialized)
	{
		printf("Attempted to create without initializing\n");
		exit(0);
	}
	ctxt = (ucontext_t*)malloc(sizeof(ucontext_t));
	if (getcontext(ctxt) == -1)
	{
		if(DEBUG)printf("Getcontext error\n");
		return -1;
	}
	char* stck =  (char*)malloc(STACK_SIZE* sizeof(char));
	ctxt->uc_stack.ss_sp =  stck;
	ctxt->uc_stack.ss_size = STACK_SIZE*sizeof(char);
	ctxt->uc_link = NULL;

	//Initialize the signal mask of each context to an empty set
	if(sigemptyset(&ctxt->uc_sigmask)<0)
	{
		return -1;
	}

	makecontext(ctxt, wrapper_function, 2, start_routine, arg);

	tcb = (gtthread_tcb*) malloc(sizeof(gtthread_tcb));
	tcb->ctxt = ctxt;

	//Assign unique thread id to each thread
	tcb->thrid = ++current_thr_id;
	*thread = tcb->thrid;
	tcb->isblocked = 0;
	tcb->iscomplete = 0;
	tcb->joinval = NULL;
	queue_init(&tcb->join_queue);
	queue_insert(&readyqueue, tcb);

	if(DEBUG)printf("Create returning\n");
	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
	return 1;
}



int  gtthread_join(gtthread_t thread, void **status)
{

	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);

	//Check if thread is in the readyqueue.
	gtthread_tcb* joinable = queue_search(&readyqueue, thread);
	if(joinable==NULL)
	{
		//Since thread isn't in the ready queue check the deletequeue
		joinable = queue_search(&deletequeue,thread);
		if(joinable!=NULL)
		{
			if(DEBUG)printf("Thread over returning\n");
			if(status!=NULL)
				*status = joinable->ret;
			sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
			return 0;

		}
	}

	//Thread is in neither queue. Return error
	if(joinable==NULL)
	{
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		return ESRCH;
	}

	//If the thread is in the ready queue. Enqueue the current thread into its join queue.
	queue_insert_normal(&joinable->join_queue,  current_thrcb);
	current_thrcb->isblocked =1;

	//Yield the processor after blocking the current thread.
	schedule_next(26);
	if(status!=NULL)
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
	//set the completed flag to indicate to the scheduler to remove the thread from readyqueue
	current_thrcb->iscomplete = 1;
	//Store the return value of the thread
	current_thrcb->ret = retval;

	if(size_of_q(&(current_thrcb->join_queue))>0)
	{
		//Unblock all threads waiting to join with the current thread and make the return value available to them
		queue_unblock_all(&(current_thrcb->join_queue), retval);
	}

	//free(retval);
	if(size_of_q(&readyqueue)>0)
	{
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		schedule_next(SIGINT);
	}
	else
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
}

int  gtthread_cancel(gtthread_t thread)
{
	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);
	int* retval = (void*) malloc(sizeof(int));

	//Set return value to threads to join with this thread to GTTHREAD_CANCELLED
	*retval = GTTHREAD_CANCELLED;
	gtthread_tcb* canceltcb = queue_search(&readyqueue, thread);
	if(canceltcb==NULL)
	{
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		return -1;
	}

	//Complete the cancelled thread and remove it from the queue immediately
	canceltcb->iscomplete = 1;
	queue_insert_normal(&deletequeue, (queue_remove(&readyqueue, thread))->thrcb);
	//queue_remove(&readyqueue, thread);
	if(DEBUG)printf("Succesfully removed from queue\n");


	//SIGMASK ALL
	if(size_of_q(&(canceltcb->join_queue))>0)
	{
		//Unblock all threads waiting on the cancelled thread
		queue_unblock_all(&(canceltcb->join_queue), (void*)retval);
	}

	canceltcb->ret = (void*) retval;
	//free(retval);

	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
	return 1;
	//gtthread_exit(retval);
}

int  gtthread_equal(gtthread_t t1, gtthread_t t2)
{
	return t1==t2;
}

int  gtthread_mutex_init(gtthread_mutex_t *mutex)
{
	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);
	//if() Check if has already been initialized.
	//mutex = (gtthread_mutex_t*) malloc(sizeof(gtthread_mutex_t));
	mutex->lock = 0;
	mutex->owner = -1;
	//mutex->destroyed = 0;
	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
	return 0;
}

int  gtthread_mutex_lock(gtthread_mutex_t *mutex)
{
	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);
	//Indicate deadlock if a thread tries to lock a mutex that it already holds.
	if(mutex->owner==current_thrcb->thrid)
	{
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		return EDEADLK;
	}

	while(mutex->lock)
	{
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		if(DEBUG)printf("Waiting for lock####################################################\n");
		//yield the processor in the case of a lock
		schedule_next(SIGINT);
	}

	mutex->lock = 1;
	if(DEBUG)printf("Acquired finally\n");
	mutex->owner = current_thrcb->thrid;
	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
	return 0;
}

int  gtthread_mutex_unlock(gtthread_mutex_t *mutex)
{
	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);

	//Generate error if the current thread doesn't own the mutex
	if(current_thrcb->thrid!=mutex->owner)
	{
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		return EPERM;
	}

	//Generate error if the mutex isn't locked
	else if(!mutex->lock)
	{
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		return EPERM;
	}
	else
	{
		mutex->lock = 0;
		mutex->owner = -1;
	}
	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
	return 0;
}



void* schedule_next(int signum)
{

	sigprocmask(SIG_BLOCK, &threadprocmask, NULL);
	if (size_of_q(&readyqueue)>1)//false this means only the main thread is in the queue
	{
		//If thread is completed remove it from the ready queue and insert into delete queue
		if(current_thrcb->iscomplete)
		{
			queue_insert_normal(&deletequeue, (queue_remove(&readyqueue, current_thrcb->thrid))->thrcb);
		}
		if(DEBUG)printf("Scheduling next thread-----------------\n");
		gtthread_tcb* tmpthr;
		tmpthr = current_thrcb;
		//Get the next thread in the ready queue
		current_thrcb = queue_next(&readyqueue);
		//If the next thread is blocked continue searching for an unblocked thread
		while(current_thrcb->isblocked)
		{
			if(DEBUG)printf("Current thread blocked: %ld\n", (long)current_thrcb->thrid);
			current_thrcb = queue_next(&readyqueue);

		}
		if(DEBUG)printf("Swapping %ld for %ld\n", tmpthr->thrid, current_thrcb->thrid);
		interval.it_value.tv_usec = timeslice;
		sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
		if(!setmain)
		{
			if(DEBUG)printf("Main has been set\n");
			setmain = 1;
			setitimer(ITIMER_VIRTUAL, &interval, NULL);
			swapcontext(&main_ctxt, current_thrcb->ctxt);

		}
		else
		{
			//set timer value explicitly every time
			setitimer(ITIMER_VIRTUAL, &interval, NULL);
			swapcontext(tmpthr->ctxt, current_thrcb->ctxt);
		}
	}

	sigprocmask(SIG_UNBLOCK, &threadprocmask, NULL);
	return (void*) 0;
}
