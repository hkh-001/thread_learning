#include <stdio.h>

#include <pthread.h>

#include <semaphore.h>

sem_t sem1;

sem_t sem2;

void *thread_a(void *in)

{

    sem_wait(&sem1);       /* wait for sem1 */

    printf("I am thread_a\n");

    pthread_exit((void *)0);

}

 

void *thread_b(void *in)

{

    sem_wait(&sem2);       /* wait for sem2 */

    printf("I am thread_b\n");

    sem_post(&sem1);       /* increase sem1 by 1, make thread_a run*/

    pthread_exit((void *)0);

}

 

void *thread_c(void *in)

{

    printf("I am thread_c\n");

    sem_post(&sem2);      /* increase sem2 by 1, make thread_b run*/

    pthread_exit((void *)0);

}

 

int main()

{

    pthread_t a,b,c;  /* thread id a, b, c*/

    int val;         /* used for function return result */

 

    /* init sem1 sem2 to 0 , any thread waits for it will be blocked*/

    sem_init(&sem1, 0, 0);

    sem_init(&sem2, 0, 0);

   

    /* create thread a, b, c */

   pthread_create(&a, NULL, thread_a, (void *)0);

   pthread_create(&b, NULL, thread_b, (void *)0);

   pthread_create(&c, NULL, thread_c, (void *)0);

 

    /* main thread waits for termination of a,b,c */

    pthread_join(a, (void **)0);

    pthread_join(b, (void **)0);

    pthread_join(c, (void **)0);

 

    /* destroy sem1 sem2 */

    sem_destroy(&sem1);

    sem_destroy(&sem2);

 

    printf("Main thread is over\n");

    return 0;

}