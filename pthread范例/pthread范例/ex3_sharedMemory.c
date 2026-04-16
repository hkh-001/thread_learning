#include<stdio.h>
#include <Windows.h>
#include<pthread.h>


//     int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
//                          void *(*start_routine) (void *), void *arg);

int g_data = 0;

void* func1(void* arg)
{
    static  char* p = "t1 is run out";
    printf("t1 :  %ld thread is created \n", pthread_self());
    printf("t1 : param is %d \n", *((int*)arg));
    
    while (1) {
        printf("t1 : %d\n", g_data++);
        Sleep(100);
    }

    pthread_exit((void*)p);
}


void* func2(void* arg)
{
    static  char* p = "t1 is run out";
    printf("t2 :  %ld thread is created \n", pthread_self());
    printf("t2 : param is %d \n", *((int*)arg));
    
    while (1) {
        printf("t2 : %d\n", g_data++);
        Sleep(100);
    }
    pthread_exit((void*)p);
}


int main()
{
    int ret;
    int param = 100;
    pthread_t t1;
    pthread_t t2;



    ret = pthread_create(&t1, NULL, func1, (void*)&param);
    if (ret == 0) {
        printf("main : create the t1 thread successed \n");
    }


    ret = pthread_create(&t2, NULL, func2, (void*)&param);
    if (ret == 0) {
        printf("main : create the t2 thread successed \n");
    }

    printf("main : %ld thread is create \n", pthread_self());


    while (1) {
        printf("main : %d\n", g_data++);
        Sleep(100);
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}

