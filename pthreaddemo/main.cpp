#include <pthread.h>
#include <stdio.h>

void* worker(void* arg)
{
    int id = *(int*)arg;
    printf("thread %d running\n", id);
    return NULL;
}

int main()
{
    pthread_t t1, t2;
    int a = 1, b = 2;

    pthread_create(&t1, NULL, worker, &a);
    pthread_create(&t2, NULL, worker, &b);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("all threads finished\n");
}