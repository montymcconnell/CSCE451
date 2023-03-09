#ifndef __WORKER_H_
#define __WORKER_H_

void cancel_thread();
void suspend_thread();
void leave_scheduler_queue(thread_info_t*);

#endif
