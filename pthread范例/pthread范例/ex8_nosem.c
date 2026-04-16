#include <stdio.h>

#include <pthread.h>

#include <semaphore.h>

 void *thread_a(void *in)

{

    printf("I am thread_a\n");

    pthread_exit((void *)0);

}

 

void *thread_b(void *in)

{

    printf("I am thread_b\n");

    pthread_exit((void *)0);

}

 

void *thread_c(void *in)

{

    printf("I am thread_c\n");

    pthread_exit((void *)0);

}

 

int main()

{

    pthread_t a,b,c;  /* thread id a, b, c*/

    int val;         /* used for function return result */

 

    /* create thread a, b, c */

    pthread_create(&a, NULL, thread_a, (void *)0);

    pthread_create(&b, NULL, thread_b, (void *)0);

    pthread_create(&c, NULL, thread_c, (void *)0);

 

    /* main thread waits for termination of a,b,c */

    pthread_join(a, (void **)0);

    pthread_join(b, (void **)0);

    pthread_join(c, (void **)0);

 

    printf("Main thread is over\n");

    return 0;

}