/*
PA3 Part 2
Monty McConnell
Producer Consumer problem with a monitor
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <iostream>

// Condition Variable Struct
typedef struct {
  int count;
  sem_t suspend;
} cond;


void *producer(void *id);
void *consumer(void *id);
void mon_insert(char alpha, int* tid);
void mon_remove(int* tid);
char generate_random_alphabet();

sem_t mutex;
cond empty;
cond full;

pthread_t pid;
pthread_t cid;

char* buffer;
int bufferLen, proThreads, conThreads, items, currentSlot;

int main (int argc, char *argv[]) {
  // Retrieve data from command line
  bufferLen = atoi(argv[2]);
  proThreads = atoi(argv[4]);
  conThreads = atoi(argv[6]);
  items = atoi(argv[8]);

  // Make list of thread id's for better readability
  int *pidList = new int[proThreads];
  int *cidList = new int[conThreads];

  // Create buffer and initialize semaphore and monitor
  buffer = (char *) malloc(sizeof(char) * bufferLen);
  sem_init(&mutex, 0, 1);
  full.count = 0;
  empty.count = bufferLen;
  sem_init(&(full.suspend), 0, 0);
  sem_init(&(empty.suspend), 0, bufferLen);
  

  // Create -p # of producer threads & # of consumer threads
  for(int i = 0; i < proThreads; i++) {
    pidList[i] = i + 1;
    pthread_create(&pid, NULL, producer, &pidList[i]);

  }
  for(int i = 0; i < conThreads; i++) {
    cidList[i] = i + 1;
    pthread_create(&cid, NULL, consumer, &cidList[i]);

  }

  // Join all threads
  for(int i = 0; i < proThreads; i++) {
    pthread_join(pid, NULL);
  }
  for(int i = 0; i < conThreads; i++) {
    pthread_join(cid, NULL);
  }

  pthread_exit(NULL);
}

void *producer(void *id) {
  
  char alpha;
  while(1) {
    int* tid = (int*)id;
    alpha = generate_random_alphabet();
    mon_insert(alpha,tid);
  }
}

void *consumer(void *id) {
  
  while(1) {
    int* tid = (int*)id;
    mon_remove(tid);
  }
}

char generate_random_alphabet(){
  char randomletter = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"[random () % 52];
  return randomletter;
}

//-------------------------------Monitor-----------------------//

void wait (cond *cv) {
  // Give up exclusive access to monitor and
  // suspend appropriate thread
  cv->count--;
  if(cv->count < 0) {
    sem_wait(&(cv->suspend));
  }
}

void signal (cond *cv) {
  // Unblock suspended thread at head of queue
  cv->count++;
  if(cv->count <= 0) {
    sem_post(&(cv->suspend));
  }
}

void mon_insert(char alpha, int* tid){
  wait(&empty);
  sem_wait(&mutex);
  // insert a random character into the first 
  // available slot in the buffer
  if(items > 0) {
    buffer[currentSlot] = alpha;
    fflush(stdout);
    std::cout<<"p:<"<<*tid<<">, item: "<<buffer[currentSlot]<<", at "<<currentSlot<<"\n";
    currentSlot++;
    items--;
  }
  else {
    sem_post(&mutex);
    signal(&full);
    pthread_exit(NULL);
  }
  sem_post(&mutex);
  signal(&full);
}

void mon_remove(int* tid){
  wait(&full);
  sem_wait(&mutex);
  // Remove character from the last used slot
  // in the buffer
  if(currentSlot > 0) {
    currentSlot--;
    char removedChar = buffer[currentSlot];
    buffer[currentSlot] = '\0';
    fflush(stdout);
    std::cout<<"c:<"<<*tid<<">, item: "<<removedChar<<", at "<<currentSlot<<"\n";
  }
  else {
    sem_post(&mutex);
    signal(&empty);
    exit(0);
  }
  sem_post(&mutex);
  signal(&empty);
}