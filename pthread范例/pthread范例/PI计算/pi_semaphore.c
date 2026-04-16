#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#ifdef WIN32

int gettimeofday(struct timeval* tp, void* tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}
#endif





int thread_count;            //thread's num
int n = 10000000000;             
double sum = 0.0;
int flag = 0;  
 
sem_t sem;

void *Thread_sum(void* rank);
int main(int argc,char* argv[]){
     struct  timeval start;
     struct  timeval end;//Use long in case of 64-bit system
     long thread;
     pthread_t* thread_handles;
    
	 gettimeofday(&start,NULL);//Get number of threads from command line

     //thread_count = strtol(argv[1],NULL,10);//将字符串转化为longint长整型

     thread_count = 10;

     thread_handles = (pthread_t*) malloc(thread_count*sizeof(pthread_t));//initialize semaphore
     
	 sem_init(&sem,0,1);
     
	 for(thread = 0;thread < thread_count;thread++){ //Create threads
           pthread_create(&thread_handles[thread],NULL,Thread_sum,(void*)thread);
     }
     printf("Hello from the main thread\n");
     for(thread = 0;thread < thread_count;thread++){ //Wait util thread_handles[thread] complete
          pthread_join(thread_handles[thread],NULL);
     }
     gettimeofday(&end,NULL);
   
    long long startusec=start.tv_sec*1000000+start.tv_usec;
    long long endusec=end.tv_sec*1000000+end.tv_usec;
    double elapsed=(double)(endusec-startusec)/1000000.0;
    printf("the result of PI took %.4f seconds\n",elapsed);
    free(thread_handles);
    
	sem_destroy(&sem); 
   
   printf("%f",4*sum);
   
   return 0;
}
void *Thread_sum(void *rank){
 long my_rank=(long)rank;
 double factor,my_sum = 0.0;
 long long i;
 long long my_n = n/thread_count;
 long long my_first_i = my_n*my_rank;
 long long my_last_i = my_first_i + my_n;
 
 if(my_first_i % 2 == 0)
     factor = 1.0;
 else
     factor = -1.0;
 for(i = my_first_i;i < my_last_i;i++,factor = -factor){
      my_sum += factor/(2*i+1);
 }//Use semaphore to solve critical sections after loop
 
 sem_wait(&sem);
  
 sum += my_sum;
 
 sem_post(&sem);


 return NULL;
}
