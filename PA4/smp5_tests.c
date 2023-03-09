/*************** YOU SHOULD NOT MODIFY ANYTHING IN THIS FILE ***************/
#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "testrunner.h"
#include "list.h"
#include "scheduler.h"
#include "worker.h"

//#define quit_if(cond) do {if (cond) exit(EXIT_FAILURE);} while(0)
#define quit_if(cond) do {if (cond) {printf("Line %d.",__LINE__);exit(EXIT_FAILURE);}} while(0)

void args_to_nums(int argc, const char **argv, int *num_workers, int *queue_size, int **quanta) {
  int i;
  quit_if(argc < 4);
  *num_workers = atoi(argv[1]);
  *queue_size = atoi(argv[2]);
  *quanta = malloc(*num_workers * sizeof(int));
  quit_if(*quanta == NULL);
  for(i=3;i<argc;i++)
    quanta[0][i-3] = atoi(argv[i]);
}

void nums_to_args(int num_workers, int queue_size, int *quanta, int *argc, char ***argv) {
  int i;
  *argc = num_workers+3;
  *argv = malloc(*argc*sizeof(char *));
  quit_if(*argv==NULL);
  argv[0][0] = "scheduler";
  argv[0][1] = malloc(3*sizeof(char));
  quit_if(argv[0][1]==NULL);
  sprintf(argv[0][1],"%d",num_workers);
  argv[0][2] = malloc(3*sizeof(char));
  quit_if(argv[0][2]==NULL);
  sprintf(argv[0][2],"%d",queue_size);
  for(i=0;i<num_workers;i++) {
    argv[0][i+3] = malloc(3*sizeof(char));
    quit_if(argv[0][i+3]==NULL);
    sprintf(argv[0][i+3],"%d",quanta[i]);
  }
  argv[0][i+3]=NULL;
}


/* Prepare input, reroute file descriptors, and run the program. */
void run_test(int argc, const char **argv)
{
  //int fork_pid = fork();
  //if (fork_pid == 0) {
  /* Reroute standard file descriptors */
  freopen("smp5.out", "w", stdout);
  /* Run the program */
  quit_if(smp5_main(argc, argv) != EXIT_SUCCESS);
  fclose(stdout);
  //} else if (fork_pid > 0) {
  //waitpid(fork_pid, 0, 0);
  //} else {
  //fprintf(stderr, "run_test: fork() error\n");
  //}
}

int test_output(FILE *stream, int nw, int qs, int *q) {
  int queue_size, queue_index;
  int num_workers, worker_index;
  int rv, in_queue, term, susp;
  unsigned long *queue, *workers, tid, prev, newwork, dummyl;
  int *remaining, *quanta;
  char dummyc;
  float tot_wait, tot_run, ave_wait, ave_run;
  int my_run, my_wait;
  rv = fscanf(stream,"Main: running %d workers with queue size %d for quanta:\n",&num_workers, &queue_size);
  quit_if(rv != 2 || num_workers != nw || queue_size != qs);
  queue = malloc(queue_size*sizeof(long));
  workers = malloc(num_workers*sizeof(long));
  quanta = malloc(num_workers*sizeof(int));
  remaining = malloc(queue_size*sizeof(int));
  for(worker_index=0;worker_index<num_workers;worker_index++) {
    quit_if(fscanf(stream, " %d", quanta+worker_index) != 1);
    quit_if(quanta[worker_index]!=q[worker_index]);
  }
  fscanf(stream,"\n");
  for(worker_index=0;worker_index<num_workers;worker_index++) {
    quit_if(fscanf(stream, "Main: detaching worker thread %lu.\n",workers+worker_index) != 1);
  }
  quit_if(fscanf(stream, "Main: waiting for scheduler %lu.\n",&dummyl) != 1);

  for(;queue_index<queue_size;queue[queue_index++]=0);


  worker_index = queue_index=0;
  in_queue = 0;

  quit_if(fscanf(stream, "Scheduler: waiting for workers.%c",&dummyc)!=1 || dummyc != '\n');
  for(queue_index = 0;queue_index < queue_size && queue_index < num_workers;queue_index++) {
    quit_if(fscanf(stream, "Thread %lu: in scheduler queue.\n",&tid)!= 1 || tid != workers[worker_index]);
    quit_if(fscanf(stream, "Thread %lu: suspending.\n",&tid)!= 1 || tid != workers[worker_index]);
    queue[queue_index]=tid;
    remaining[queue_index] = quanta[worker_index];
    worker_index++;
    in_queue++;
  }
  my_run=0;
  my_wait = num_workers;
  queue_index = 0;
  term = susp = 0;
  while(worker_index < num_workers || in_queue > 0) {
    while(!queue[queue_index])
      queue_index= (queue_index+1)%queue_size;
    quit_if(fscanf(stream, "Scheduler: scheduling.%c",&dummyc)!=1 || dummyc != '\n');
    quit_if(fscanf(stream, "Scheduler: resuming %lu.\n",&tid) != 1);
    quit_if( tid != queue[queue_index]);
    if (prev == tid) {
      if(term) {
	quit_if(fscanf(stream, "Thread %lu: terminating.\n",&tid) != 1 || tid != prev);
      } else if (susp){
	quit_if(fscanf(stream, "Thread %lu: suspending.\n",&tid) != 1);
	quit_if( tid != prev);
      }
      quit_if(fscanf(stream, "Thread %lu: resuming.\n",&tid) != 1);
      quit_if(tid != queue[queue_index]);
    } else {
      quit_if(fscanf(stream, "Thread %lu: resuming.\n",&tid) != 1 || tid != queue[queue_index]);
      if(term) {
	if(queue_size == 1)
	  quit_if(fscanf(stream, "Scheduler: waiting for workers.%c",&dummyc)!=1 || dummyc!='\n');
	quit_if(fscanf(stream, "Thread %lu: terminating.\n",&tid) != 1 || tid != prev);
	if (in_queue == queue_size) {
	  quit_if(fscanf(stream, "Thread %lu: in scheduler queue.\n",&tid)!=1||tid != newwork);
	  quit_if(fscanf(stream, "Thread %lu: suspending.\n",&tid)!=1 || tid!=newwork);
	}
      } else if (susp && in_queue>1){
	quit_if(fscanf(stream, "Thread %lu: suspending.\n",&tid) != 1);
	quit_if( tid != prev);
	prev = tid;
      }
    }
    quit_if(fscanf(stream, "Scheduler: suspending %lu.\n",&tid) != 1);
    quit_if(tid != queue[queue_index]);
    if(!--remaining[queue_index]) {
      quit_if(fscanf(stream, "Thread %lu: leaving scheduler queue.\n",&tid)!=1 || tid != queue[queue_index]);
      term = 1;
      if(worker_index < num_workers) {
	
	queue[queue_index] = workers[worker_index];
	remaining[queue_index] = quanta[worker_index];
	newwork = workers[worker_index];
	worker_index++;
	if(queue_size == 1) {
	  prev = tid;
	  quit_if(fscanf(stream, "Scheduler: waiting for workers.%c",&dummyc)!=1 || dummyc!='\n');
	  quit_if(fscanf(stream, "Thread %lu: terminating.\n",&tid) != 1 || tid != prev);
	  quit_if(fscanf(stream, "Thread %lu: in scheduler queue.\n",&tid)!= 1 || tid != newwork);
	  quit_if(fscanf(stream, "Thread %lu: suspending.\n",&tid)!= 1 || tid != newwork);
	  term = 0;
	  susp = 0;
	  my_wait++;
	}
      } else {
	queue[queue_index] = 0;
	in_queue--;
      }
    } else {
      term = 0;
      susp = 1;
    }
    prev = tid;
    my_run++;
    my_wait += in_queue+(num_workers-worker_index)-1+term;
    queue_index= (queue_index+1)%queue_size;
  } 
  quit_if(fscanf(stream, "Th%c",&dummyc) != 1);
  if (dummyc=='r') {
    quit_if(fscanf(stream, "ead %lu: terminating.\nThe",&tid)!=1 || tid != prev);
  }
  quit_if(fscanf(stream, " total wait time is %f seconds.\n",&tot_wait) != 1);
  quit_if(fscanf(stream, "The total run time is %f seconds.\n",&tot_run) != 1);
  quit_if(fscanf(stream, "The average wait time is %f seconds.\n",&ave_wait) != 1);
  quit_if(fscanf(stream, "The average run time is %f seconds.\n",&ave_run) != 1);
  if (dummyc=='e')
    quit_if(fscanf(stream, "Thread %lu: terminating.\nThe",&tid) != 1|| tid != prev);
  quit_if(abs(tot_wait-my_wait)>1);
  quit_if(abs(tot_run-my_run)>1);
  quit_if(abs(tot_wait/num_workers-ave_wait)>.5);
  quit_if(abs(tot_run/num_workers-ave_run)>.5);
  return 0;
}

int general_test(int argc, const char **argv) {
  FILE *f;
  int nw, qs, *q;
  // Justin Bradley fixed by moving args_to_nums call before run_test (duh)
  args_to_nums(argc,argv,&nw,&qs,&q);
  run_test(argc,argv);
  f = fopen("smp5.out","r");
  test_output(f,nw,qs,q);
  return EXIT_SUCCESS;
}

int specific_test(int nw, int qs, int *q) {
  FILE *f;
  int argc;
  char **argv;
  nums_to_args(nw,qs,q,&argc,&argv);
  run_test(argc,(const char **)argv);
  f = fopen("smp5.out","r");
  test_output(f,nw,qs,q);
  return EXIT_SUCCESS;
}  


int test_3_1_2_2_2() {
  int q[3] = {2,2,2};
  return specific_test(3,1,q);
}  

int test_2_2_2_2() {
  int q[2]={2,2};
  return specific_test(2,2,q);
}  

int test_5_7_1_2_1_2_1() {
  int q[5] = {1,2,1,2,1};
  return specific_test(5,7,q);
}

int test_4_1_1_2_3_4() {
  int q[4] = {1,2,3,4};
  return specific_test(4,1,q);
}

int test_3_3_4_3_2() {
  int q[3] = {4,3,2};
  return specific_test(3,3,q);
}

/*
 * Main entry point for SMP% test harness
 */
int run_smp5_tests(int argc, const char **argv)
{
	/* Tests can be invoked by matching their name or their suite name
	 * or 'all' */
	testentry_t tests[] = {
	  {"test_3_1_2_2_2",  "rr", test_3_1_2_2_2}, 
	  {"test_2_2_2_2",  "rr", test_2_2_2_2}, 
	  {"test_5_7_1_2_1_2_1",  "rr", test_5_7_1_2_1_2_1}, 
	  {"test_4_1_1_2_3_4",  "rr", test_4_1_1_2_3_4},
	  {"test_3_3_4_3_2",  "rr", test_3_3_4_3_2},
	  {"general",  "gen", general_test}
	};
	int result = run_testrunner(argc, argv, tests, sizeof(tests) / sizeof(testentry_t));
	unlink("smp5.out");
	return result;
}

/* The real main function.  */
int main(int argc, const char **argv)
{
	if (argc > 1 && !strcmp(argv[1], "-test")) {
		return run_smp5_tests(argc - 1, argv + 1);
	} else {
		return smp5_main(argc, argv);
	}
}
