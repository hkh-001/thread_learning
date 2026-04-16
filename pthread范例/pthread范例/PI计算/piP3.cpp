#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <iomanip>
#include <time.h>
#include <pthread.h>

#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif /* _UNISTD_H */



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

using namespace std;

long long n = 10000000000;
const int thread_count = 10;
double sum = 0.0;
pthread_mutex_t mutexsum, lock;

void* Thread_sum(void* rank)
{
    int my_rank = *(int *) rank;
    double factor, my_sum = 0.0;
    long long i;
    long long my_n = n/thread_count;
    long long my_first_i = my_n * my_rank;
    long long my_last_i = my_first_i + my_n;

    if(my_first_i % 2 == 0) /* my_first_i is even */
        factor = 1.0;
    else    /* my_first_i is odd */
        factor = -1.0;
    
    for(i = my_first_i; i < my_last_i; i ++, factor = -factor)
        my_sum += factor/(2*i+1);
    
    pthread_mutex_lock(&mutexsum);
    sum += my_sum;
    pthread_mutex_unlock(&mutexsum);
    
    return NULL;
    /* Thread_sum */
}

int main(void)
{
    struct timeval time1, time2;
    pthread_mutex_init(&mutexsum, NULL);
    
    gettimeofday(&time1, NULL);
    
    pthread_t thread_ID[thread_count];
    
    int value[thread_count];
    for(int i = 0; i < thread_count; i ++)
        value[i] = i;
    
    //Create the thread, passing &value for the argument.
    for(int i = 0; i < thread_count; i ++)
        pthread_create(&thread_ID[i], NULL, Thread_sum, &value[i]);
    
    //Wait for the thread to terminate.
    for(int i = 0; i < thread_count; i ++)
        pthread_join(thread_ID[i], NULL);
    
    sum *= 4.0;
    printf("%.20lf\n", sum);
    gettimeofday(&time2, NULL);
    printf("s: %ld, ms: %ld\n", time2.tv_sec-time1.tv_sec, (time2.tv_sec*1000 + time2.tv_sec/1000)-(time1.tv_sec*1000 + time1.tv_sec/1000));
    
    return 0;
}
