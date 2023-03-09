#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "scheduler.h"
#include "worker.h"

/*
 * define the extern global variables here.
 */
sem_t		queue_sem;	/* semaphore for scheduler queue */
thread_info_list sched_queue; /* list of current workers */

static int quit = 0;
static timer_t timer;
static thread_info_t *currentThread= 0;
static long wait_times;
static long run_times;
static int completed = 0;
static int thread_count = 0;

static void exit_error(int); /* helper function. */
static void wait_for_queue();

/*******************************************************************************
 *
 * Implement these functions.
 *
 ******************************************************************************/
/*
 * This function intializes the queue semaphore and the queue itself.
 */

/* 
 * Update the worker's current running time.
 * This function is called every time the thread is suspended.
 */
void update_run_time(thread_info_t *info) {
        /* TODO: implement this function */
	clock_gettime(CLOCK_REALTIME,&info->suspend_time);
	info->run_time += time_difference(&info->suspend_time, &info->resume_time);
}

/* 
 * Update the worker's current waiting time.
 * This function is called every time the thread resumes.
 */
void update_wait_time(thread_info_t *info) {
        /* TODO: implement this function */
	clock_gettime(CLOCK_REALTIME,&info->resume_time);
	info->wait_time += time_difference(&info->resume_time,&info->suspend_time);
}



static void init_sched_queue(int queue_size)
{
	/* set up a semaphore to restrict access to the queue */
	sem_init(&queue_sem, 0, queue_size);

	/* initialize the scheduler queue */
	sched_queue.head = sched_queue.tail = 0;
	pthread_mutex_init(&sched_queue.lock, NULL);

	/* TODO: initialize the timer */
	timer_create(CLOCK_REALTIME, NULL, &timer);
}

/*
 * signal a worker thread that it can resume.
 */
static void resume_worker(thread_info_t *info)
{
	printf("Scheduler: resuming %lu.\n", info->thrid);

	/*
	 * TODO: signal the worker thread that it can resume 
	 */
	pthread_kill(info->thrid, SIGUSR2);
	/* update the wait time for the thread */
	update_wait_time(info);

}

/*send a signal to the thread, telling it to kill itself*/
void cancel_worker(thread_info_t *info)
{

	/* TODO: send a signal to the thread, telling it to kill itself*/
	pthread_kill (info->thrid, SIGTERM);
	/* Update global wait and run time info */
	wait_times += info->wait_time;
	run_times += info->run_time;
	completed++;

	/* Update schedule queue */
	leave_scheduler_queue(info);

	if (completed >= thread_count) {
  	    sched_yield(); /* Let other threads terminate. */
		printf("The total wait time is %f seconds.\n", (float)wait_times / 1000000);
		printf("The total run time is %f seconds.\n", (float)run_times / 1000000);
		printf("The average wait time is %f seconds.\n", (float)wait_times / 1000000 / thread_count);
		printf("The average run time is %f seconds.\n", (float)run_times / 1000000 / thread_count);
	}
}

/*
 * signals a worker thread that it should suspend.
 */
static void suspend_worker(thread_info_t *info)
{

        // int whatgoeshere = 0;
	printf("Scheduler: suspending %lu.\n", info->thrid);

	/*update the run time for the thread*/
	update_run_time(info);

	/* TODO: Update quanta remaining. */
	info->quanta--;
	/* TODO: decide whether to cancel or suspend thread */
	if(info->quanta > 0) {
	  /*
	   * Thread still running: suspend.
	   * TODO: Signal the worker thread that it should suspend.
	   */
	  pthread_kill(info->thrid, SIGUSR1);
	  /* Update Schedule queue */
	  list_remove(&sched_queue,info->le);
	  list_insert_tail(&sched_queue,info->le);
	} else {
	  /* Thread done: cancel */
	  cancel_worker(info);
	}
}

/*
 * this is the scheduling algorithm
 * pick the next worker thread from the available list
 * you may need to add new information to the thread_info struct
 */
static thread_info_t *next_worker()
{
	if (completed >= thread_count)
		return 0;

	wait_for_queue();
	printf("Scheduler: scheduling.\n");

	/* return the thread_info_t for the next thread to run */
	return sched_queue.head->info;
}

void timer_handler()
{
	thread_info_t *info = 0;

	/* once the last worker has been removed, we're done. */
	if (list_size(&sched_queue) == 0) {
		quit = 1;
		return;
	}

	/*suspend the current worker*/
	if (currentThread)
		suspend_worker(currentThread);

	//resume the next worker 
	info = next_worker();

	/* Update currentThread */
	currentThread = info;  

	if (info)
		resume_worker(info);
	else
		quit = 1;
}

/* 
 * Set up the signal handlers for SIGALRM, SIGUSR1, and SIGTERM.
 * TODO: Implement this function.
 */
void setup_sig_handlers() {

	/* Setup timer handler for SIGALRM signal in scheduler */
	struct sigaction sa_alrm;
	sa_alrm.sa_handler = timer_handler;
	sigemptyset(&sa_alrm.sa_mask);
	sa_alrm.sa_flags = 0;
	sigaction(SIGALRM, &sa_alrm, NULL); 

	/* Setup cancel handler for SIGTERM signal in workers */
	struct sigaction sa_term;
	sa_term.sa_handler = cancel_thread;
	sigemptyset(&sa_term.sa_mask);
	sa_term.sa_flags = 0;
	sigaction(SIGTERM, &sa_term, NULL);
	
	/* Setup suspend handler for SIGUSR1 signal in workers */
	struct sigaction sa_usr1;
	sa_usr1.sa_handler = suspend_thread;
	sigemptyset(&sa_usr1.sa_mask);
	sa_usr1.sa_flags = 0;
	sigaction(SIGUSR1, &sa_usr1, NULL);
}

/*******************************************************************************
 *
 * 
 *
 ******************************************************************************/

/*
 * waits until there are workers in the scheduling queue.
 */
static void wait_for_queue()
{

	while(!list_size(&sched_queue)) {
	  printf("Scheduler: waiting for workers.\n");
	  sched_yield();
	}
}

/*
 * runs at the end of the program just before exit.
 */
static void clean_up()
{
	/*
	 * destroy any mutexes/condition variables/semaphores that were created.
	 * free any malloc'd memory not already free'd
	 */
	sem_destroy(&queue_sem);
	pthread_mutex_destroy(&sched_queue.lock);
}

/*
 * prints the program help message.
 */
static void print_help(const char *progname)
{
	printf("usage: %s <num_threads> <queue_size> <i_1, i_2 ... i_numofthreads>\n", progname);
	printf("\tnum_threads: the number of worker threads to run\n");
	printf("\tqueue_size: the number of threads that can be in the scheduler at one time\n");
	printf("\ti_1, i_2 ...i_numofthreads: the number of quanta each worker thread runs\n");
}

/*
 * prints an error summary and exits.
 */
static void exit_error(int err_num)
{
	fprintf(stderr, "failure: %s\n", strerror(err_num));
	exit(1);
}

/*
 * creates the worker threads.
 */
static void create_workers(int thread_count, int *quanta)
{
	int i = 0;
	int err = 0;

	for (i = 0; i < thread_count; i++) {
		thread_info_t *info = (thread_info_t *) malloc(sizeof(thread_info_t));
		info->quanta = quanta[i];

		if ((err = pthread_create(&info->thrid, NULL, start_worker, (void *)info)) != 0) {
			exit_error(err);
		}
		printf("Main: detaching worker thread %lu.\n", info->thrid);
		pthread_detach(info->thrid);

		/* TODO: initialize the time variables for each thread for performance evalution*/
		info->run_time = 0;
		info->wait_time = 0;
		clock_gettime(CLOCK_REALTIME,&info->suspend_time);
		clock_gettime(CLOCK_REALTIME,&info->resume_time);
	}
}

/*
 * runs the scheduler.
 */
static void *scheduler_run(void *unused)
{
	struct itimerspec new_value;
	new_value.it_value.tv_sec = QUANTUM;
	new_value.it_value.tv_nsec = 0;
	new_value.it_interval.tv_sec = QUANTUM;
	new_value.it_interval.tv_nsec = 0;
	wait_for_queue();

	/* TODO: start the timer */
	timer_settime(timer, 0, &new_value, NULL);
	/*keep the scheduler thread alive*/
	while( !quit )
		sched_yield();

	return NULL;
}

/*
 * starts the scheduler.
 * returns 0 on success or exits program on failure.
 */
static int start_scheduler(pthread_t *thrid)
{
	int err = 0;

	if ((err = pthread_create(thrid, NULL, scheduler_run, 0)) != 0) {
		exit_error(err);
	}

	return err;
}

/*
 * reads the command line arguments and starts the scheduler & worker threads.
 */
int smp5_main(int argc, const char** argv)
{
	int queue_size = 0;
	int ret_val = 0;
	int *quanta,i;
	pthread_t sched_thread;

	/* check the arguments. */
	if (argc < 3) {
		print_help(argv[0]);
		exit(0);
	}

	thread_count = atoi(argv[1]);
	queue_size = atoi(argv[2]);
	quanta = (int*)malloc(sizeof(int)*thread_count);
	if (argc != 3 + thread_count) {
		print_help(argv[0]);
		exit(0);
	}

	for ( i = 0; i < thread_count; i++)
		quanta[i] = atoi(argv[i+3]);

	printf("Main: running %d workers with queue size %d for quanta:\n", thread_count, queue_size);
	for ( i = 0; i < thread_count; i++)
		printf(" %d", quanta[i]);
	printf("\n");

	/*setup the sig handlers for scheduler and workers*/
	setup_sig_handlers();

	/* initialize anything that needs to be done for the scheduler queue. */
	init_sched_queue(queue_size);

	/* creates a thread for the scheduler. */
	start_scheduler(&sched_thread);

	/* creates the worker threads and returns. */
	create_workers(thread_count, quanta);

	/* wait for scheduler to finish */
	printf("Main: waiting for scheduler %lu.\n", sched_thread);
	pthread_join(sched_thread, (void **) &ret_val);

	/* clean up our resources */
	clean_up();

	/* this will wait for all other threads */
	pthread_exit(0);
}

long time_difference(const struct timespec *time1, const struct timespec *time2) {
	return (time1->tv_sec - time2->tv_sec) * 1000000 + (time1->tv_nsec - time2->tv_nsec) / 1000;
}
