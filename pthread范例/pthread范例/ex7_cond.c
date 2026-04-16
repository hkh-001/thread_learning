/*
条件变量 Condition Variable
互斥锁只有两种状态，这限制了它的用途. 条件变量允许线程在阻塞的时候等待另一个线程发送的信号，
当收到信号后，阻塞的线程就被唤醒并试图锁定与之相关的互斥锁. 条件变量要和互斥锁结合使用。

条件变量的声明和初始化
通过声明 pthread_cond_t 类型的数据,并且必须先初始化才能使用。

初始化的方法也有两种：

第一种，利用内部定义的常量，例如：

pthread_cond_t myconvar = PTHREAD_COND_INITIALIZER;
第二种，利用函数 pthread_cond_init(cond,attr)，
其中 attr 由 pthread_condattr_init() 和 pthread_condattr_destroy() 创建和销毁。

可以用 pthread_cond_destroy() 销毁一个条件变量。

相关函数：

pthread_cond_wait (condition,mutex);
pthread_cond_signal (condition);
pthread_cond_broadcast (condition);
pthread_cond_wait() 会阻塞调用它的线程，直到收到某一信号。
这个函数需要在 mutex 已经被锁之后进行调用，并且当线程被阻塞时，
会自动解锁 mutex。信号收到后，线程被唤醒，这时 mutex 又会被这个线程锁定。

pthread_cond_signal() 函数结束时，必须解锁 mutex，以供 pthread_cond_wait() 锁定mutex。

当不止一个线程在等待信号时，要用 pthread_cond_broadcast() 
代替 pthread_cond_signal() 来告诉所有被该条件变量阻塞的线程结束阻塞状态。

示例

下面是一个例子，三个线程共同访问 count 变量，
thread 2 和 thread 3 竞争地对其进行加 1 的操作，
thread 1 等 count 达到 12 的时候，一次性加 125 。 
然后 thread 2 和 thread 3 再去竞争 count 的控制权，
直到完成自己的对 count 加 10 次的任务。

*/




#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include<Windows.h>

#define NUM_THREADS  3
#define TCOUNT 10
#define COUNT_LIMIT 12

int count = 0;
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;

void* inc_count(void* t)
{
    int i;
    long my_id = (long)t;

    for (i = 0; i < TCOUNT; i++)
    {
        pthread_mutex_lock(&count_mutex);
        count++;

        /*
        Check the value of count and signal waiting thread when condition is
        reached.  Note that this occurs while mutex is locked.
        */
        if (count == COUNT_LIMIT)
        {
            printf("inc_count(): thread %ld, count = %d  Threshold reached. ",
                my_id, count);
            pthread_cond_signal(&count_threshold_cv);
            printf("Just sent signal.\n");
        }
        printf("inc_count(): thread %ld, count = %d, unlocking mutex\n",
            my_id, count);
        pthread_mutex_unlock(&count_mutex);

        /* Do some work so threads can alternate on mutex lock */
        Sleep(1);
    }
    pthread_exit(NULL);
}

void* watch_count(void* t)
{
    long my_id = (long)t;

    printf("Starting watch_count(): thread %ld\n", my_id);

    /*
    Lock mutex and wait for signal.  Note that the pthread_cond_wait routine
    will automatically and atomically unlock mutex while it waits.
    Also, note that if COUNT_LIMIT is reached before this routine is run by
    the waiting thread, the loop will be skipped to prevent pthread_cond_wait
    from never returning.
    */
    pthread_mutex_lock(&count_mutex);
    while (count < COUNT_LIMIT)
    {
        printf("watch_count(): thread %ld Count= %d. Going into wait...\n", my_id, count);
        pthread_cond_wait(&count_threshold_cv, &count_mutex);
        printf("watch_count(): thread %ld Condition signal received. Count= %d\n", my_id,
            count);
        printf("watch_count(): thread %ld Updating the value of count...\n", my_id, count);
        count += 125;
        printf("watch_count(): thread %ld count now = %d.\n", my_id, count);
    }
    printf("watch_count(): thread %ld Unlocking mutex.\n", my_id);
    pthread_mutex_unlock(&count_mutex);
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    int i, rc;
    long t1 = 1, t2 = 2, t3 = 3;
    pthread_t threads[3];
    pthread_attr_t attr;

    /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init(&count_threshold_cv, NULL);

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&threads[0], &attr, watch_count, (void*)t1);
    pthread_create(&threads[1], &attr, inc_count, (void*)t2);
    pthread_create(&threads[2], &attr, inc_count, (void*)t3);

    /* Wait for all threads to complete */
    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    printf("Main(): Waited and joined with %d threads. Final value of count = %d. Done.\n",
        NUM_THREADS, count);

    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&count_threshold_cv);
    pthread_exit(NULL);

}