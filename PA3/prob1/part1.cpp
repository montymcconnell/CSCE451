/*
PA3 Part 1
Monty McConnell
----Producer Consumer problem with semaphores----
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>

void *producer (void *id);
void *consumer (void *id);

sem_t mutex;
sem_t empty;
sem_t full;

pthread_t pid;
pthread_t cid;

char* buffer;
int bufferLen, proThreads, conThreads, items, currentSlot;

int main( int argc, char *argv[] )
{
  if(argc != 9){
      printf("Invalid arguments\n");
      return -1;
  }
  // Retrieve data from command line
  bufferLen = atoi(argv[2]);
  proThreads = atoi(argv[4]);
  conThreads = atoi(argv[6]);
  items = atoi(argv[8]);

  // List of ID's for better readability
  int *pidList = new int[proThreads];
  int *cidList = new int[conThreads];
    
  buffer = (char *) malloc(sizeof(char) * bufferLen);
  currentSlot = 0;
    
  sem_init(&mutex, 0, 1);
  sem_init(&full, 0, 0);
  sem_init(&empty, 0, bufferLen);

  // create -p # of producer threads
  for(int i = 0; i < proThreads; i++) {
      pidList[i] = i + 1;
      pthread_create(&pid, NULL, producer, &pidList[i]);
  }

  // create -c # of consumer threads
  for(int i = 0; i < conThreads; i++) {
      cidList[i] = i + 1;
      pthread_create(&cid, NULL, consumer, &cidList[i]);
  }

  pthread_exit(NULL);
}


void * producer( void *id)
{
  while(1) {
    sem_wait(&empty);
    sem_wait(&mutex);
    // Insert X into the first available slot in the buffer 
    if(items > 0){
      int* tid = (int*)id;
      buffer[currentSlot] = 'X';
      std::cout<<"p:<"<<*tid<<">, item: "<<buffer[currentSlot]<<", at "<<currentSlot<<"\n";
      currentSlot++;
      items--;
    }
    
    sem_post(&mutex);
    sem_post(&full);
  } 
}

void * consumer( void *id)
{
  while(1) {
    sem_wait(&full);
    sem_wait(&mutex);
    // Remove X from the last used slot in the buffer
    if(currentSlot > 0){
      int* tid = (int*)id;
      currentSlot--;
      char removedItem = buffer[currentSlot];
      buffer[currentSlot] = '\0';
      std::cout<<"c:<"<<*tid<<">, item: "<<removedItem<<", at "<<currentSlot<<"\n";
    }
    else{
      sem_post(&mutex);
      sem_post(&empty);
      exit(0);
    }
    sem_post(&mutex);
    sem_post(&empty);
  }
}