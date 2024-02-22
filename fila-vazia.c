/* File:  
 *    pth_pool.c
 *
 * Purpose:
 *    Implementação de um pool de threads
 *
 *
 * Compile:  gcc -g -Wall -o prod-cons prod-cons.c -lpthread -lrt
 * Usage:    ./prod-cons
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define THREAD_NUM 3    // Tamanho do pool de threads
#define BUFFER_SIZE 256 // Número máximo de clocks enfileirados

typedef struct Clock { 
  int p[3];
} Clock;


Clock clockQueue[BUFFER_SIZE]; // Lista de clocks
int clockCount = 0;

pthread_mutex_t mutex; // define mutex

pthread_cond_t condFull; // declara condição
pthread_cond_t condEmpty; // declara condição

void printConsumeClock(Clock* clock, int id) {
   printf("Thread %d consumed Clock: (%d, %d, %d)\n", id, clock->p[0], clock->p[1], clock->p[2]);
   
}

void printSubmitClock(Clock* clock, int id) {
   printf("Thread %d submitted Clock: (%d, %d, %d)\n", id, clock->p[0], clock->p[1], clock->p[2]);
}

Clock getClock(){
   pthread_mutex_lock(&mutex);
   
   while (clockCount == 0){
      printf("Clock count: EMPTY\n");
      pthread_cond_wait(&condEmpty, &mutex);
   }
   
   Clock clock = clockQueue[0];
   int i;
   for (i = 0; i < clockCount - 1; i++){
      clockQueue[i] = clockQueue[i+1];
   }
   clockCount--;
   
   pthread_mutex_unlock(&mutex);
   pthread_cond_signal(&condFull);
   return clock;
}

void submitClock(Clock clock){
   pthread_mutex_lock(&mutex);

   while (clockCount == BUFFER_SIZE){
      pthread_cond_wait(&condFull, &mutex);
   }

   clockQueue[clockCount] = clock;
   clockCount++;

   pthread_mutex_unlock(&mutex);
   pthread_cond_signal(&condEmpty);
}

void *startThreadsConsumidoras(void* args);  
void *startThreadsProdutoras(void* args);  

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   pthread_mutex_init(&mutex, NULL);
   
   pthread_cond_init(&condEmpty, NULL);
   pthread_cond_init(&condFull, NULL);

   pthread_t thread[THREAD_NUM]; 
   srand(time(NULL));
   long i;
   
   for (i = 3; i < THREAD_NUM*2; i++){  
      if (pthread_create(&thread[i], NULL, &startThreadsProdutoras, (void*) i) != 0) {
         perror("Failed to create the thread");
      }  
   }
   
   for (i = 0; i < THREAD_NUM; i++){  
      if (pthread_create(&thread[i], NULL, &startThreadsConsumidoras, (void*) i) != 0) {
         perror("Failed to create the thread");
      }  
   }
   
   for (i = 0; i < THREAD_NUM*2; i++){  
      if (pthread_join(thread[i], NULL) != 0) {
         perror("Failed to join the thread");
      }  
   }
   
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&condEmpty);
   pthread_cond_destroy(&condFull);
   return 0;
}  /* main */

/*-------------------------------------------------------------------*/
void *startThreadsConsumidoras(void* args) {
   long id = (long) args; 
   while (1){ 
      Clock clock = getClock();
      printConsumeClock(&clock, id);
      sleep(1);
   }
   return NULL;
} 

void *startThreadsProdutoras(void* args) {
   long id = (long) args;
   while (1){
      sleep(2);
      Clock c = {{rand() % 10, rand() % 10, rand() % 10 }};
      submitClock(c);
      printSubmitClock(&c, id);
   }
   return NULL;
} 

