#include <stdio.h>
#include <stdlib.h>
//pthread 线程库的头文件
#include <pthread.h>

//定义线程数量
int thread_count = 4;
void* Hello(void* rank);//负载函数
int main(int argc, char* argv[]) {
    pthread_t* thread_handles;
    thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thread_handles[i], NULL, Hello, (void*)i);
    }

    printf("Hello from the main thread\n");
    for (int i = 0; i < thread_count; i++)
        pthread_join(thread_handles[i], NULL);
    free(thread_handles);
    return 0;
}

void* Hello(void* rank) {
    long my_rank = (long)rank;
    printf("Hello from thread %ld of %d\n", my_rank, thread_count);
    return NULL;

}