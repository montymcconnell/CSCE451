#ifndef __SCHEDULER_H_
#define __SCHEDULER_H_

#include <pthread.h>
#include <semaphore.h>
#include "list.h"

#define QUANTUM 1 

/* typedefs */
typedef struct thread_info {
	pthread_t		thrid;
        int                     quanta;
  	list_elem*		le;
	/*added for evalution bookkeeping*/
	struct timespec suspend_time;
	struct timespec resume_time;
	long wait_time;
	long run_time;
} thread_info_t;

/* functions */
void *start_worker(void *);
long time_difference(const struct timespec*, const struct timespec*);
int smp5_main(int argc, const char** argv);

/* shared variables */
extern sem_t		queue_sem;	/* semaphore for scheduler queue */
extern thread_info_list sched_queue;	/* list of current workers */

#endif /* __SCHEDULER_H_ */
