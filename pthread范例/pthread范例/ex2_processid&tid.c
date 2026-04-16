#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"


/** This file is part of the Mingw32 package.

unistd.h maps (roughly) to io.h
*/
#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif /* _UNISTD_H */

#define NUM_THREADS 4

long int thread_count;

void* Thread_func(void* rank);

int main(int argc,char* argv[])
{
   pid_t pid;
   pthread_t tid[NUM_THREADS];
   pid = _getpid();
   printf("Process id = %d\n",pid);
   
   long int thread;  
   thread_count = NUM_THREADS;  
   
 
   
   for(thread=0;thread<thread_count;thread++)
       pthread_create(&tid[thread],NULL,Thread_func,NULL);
   

   for(thread=0;thread<thread_count;thread++)
       pthread_join(tid[thread],NULL);
  
   
   
   return 0;
}

void* Thread_func(void* rank)
{
   
   printf("Thread id = %ld\n",pthread_self());
   
   pthread_exit(NULL);
}