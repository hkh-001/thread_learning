/*
对线程的阻塞（Joining and Detaching Threads）
阻塞是线程之间同步的一种方法。

int pthread_join(pthread_t threadid, void **value_ptr);
pthread_join 函数会让调用它的线程等待 threadid 线程运行结束之后再运行。

value_ptr 存放了其他线程的返回值。

一个可以被join的线程，仅仅可以被别的一个线程 join，
如果同时有多个线程尝试 join 同一个线程时，最终结果是未知的。
另外，线程不能 join 自己。

上面提到过，创建一个线程时，要赋予它一定的属性，
这其中就包括joinable or detachable 的属性，只有被声明成joinable的线程，可以被其他线程join。

POSIX标准的最终版本指出线程应该被设置成joinable的。显式地设置一个线程为joinable，
需要以下四个步骤：

Declare a pthread attribute variable of the pthread_attr_t data type
Initialize the attribute variable with pthread_attr_init()
Set the attribute detached status with pthread_attr_setdetachstate()
When done, free library resources used by the attribute with pthread_attr_destroy()
        值得注意的的是：僵尸线程 ( “zombie” thread )是一种已经退出了的 joinable 的线程，
        但是等待其他线程调用 pthread_join 来 join 它，以收集它的退出信息(exit status)。
        如果没有其他线程调用 pthread_join 来 join 它的话，它占用的一些系统资源不会被释放，
        比如堆栈。如果main()函数需要长时间运行，并且创建大量 joinable 的线程，
        就有可能出现堆栈不足的 error 。

        对于那些不需要 join 的线程，最好利用 pthread_detach，这样它运行结束后，
        资源就会及时得到释放。注意一个线程被使用 pthread_detach 之后，它就不能再被改成 joinable 的了。

        总而言之，创建的每一个线程都应该使用 pthread_join 或者 pthread_detach 其中一个，
        以防止僵尸线程的出现。

相关函数：

pthread_detach (threadid)
pthread_attr_setdetachstate (attr,detachstate)
pthread_attr_getdetachstate (attr,detachstate)
*/



/*
补充内容：
        在任何一个时间点上，线程是可结合的（joinable），或者是分离的（detached）。
        一个可结合的线程能够被其他线程收回其资源和杀死；在被其他线程回收之前，
        它的存储器资源（如栈）是不释放的。相反，一个分离的线程是不能被其他线程回收或杀死的，
        它的存储器资源在它终止时由系统自动释放。

        线程的分离状态决定一个线程以什么样的方式来终止自己。
        在默认情况下线程是非分离状态的，这种情况下，原有的线程等待创建的线程结束。
        只有当pthread_join（）函数返回时，创建的线程才算终止，才能释放自己占用的系统资源。
        而分离线程不是这样子的，它没有被其他的线程所等待，自己运行结束了，线程也就终止了，
        马上释放系统资源。程序员应该根据自己的需要，选择适当的分离状态。
        所以如果我们在创建线程时就知道不需要了解线程的终止状态，
        则可以pthread_attr_t结构中的detachstate线程属性，让线程以分离状态启动。

        设置线程分离状态的函数为：

// 第二个参数可选为：
// PTHREAD_CREATE_DETACHED（分离线程）
// PTHREAD _CREATE_JOINABLE（非分离线程）
pthread_attr_setdetachstate（pthread_attr_t *attr, int detachstate）

        这里要注意的一点是，如果设置一个线程为分离线程，而这个线程运行又非常快，
        它很可能在pthread_create函数返回之前就终止了，
        它终止以后就可能将线程号和系统资源移交给其他的线程使用，
        这样调用pthread_create的线程就得到了错误的线程号。
        要避免这种情况可以采取一定的同步措施，
        最简单的方法之一是可以在被创建的线程里调用pthread_cond_timewait函数，
        让这个线程等待一会儿，留出足够的时间让函数pthread_create返回。
        设置一段等待时间，是在多线程编程里常用的方法。
        但是注意不要使用诸如wait（）之类的函数，它们是使整个进程睡眠，并不能解决线程同步的问题。

        另外一个可能常用的属性是线程的优先级，它存放在结构sched_param中。
        用函数pthread_attr_getschedparam和函数pthread_attr_setschedparam进行存放，
        一般说来，我们总是先取优先级，对取得的值修改后再存放回去。

线程等待——正确处理线程终止

#include <pthread.h>

void pthread_exit(void *retval);

void pthread_join(pthread_t th,void *thread_return);//挂起等待th结束,*thread_return=retval;

int pthread_detach(pthread_t th);
        如果线程处于joinable状态，则只能只能被创建他的线程等待终止。

        在Linux平台默认情况下，虽然各个线程之间是相互独立的，
        一个线程的终止不会去通知或影响其他的线程。
        但是已经终止的线程的资源并不会随着线程的终止而得到释放，
        我们需要调用 pthread_join() 来获得另一个线程的终止状态并且释放该线程所占的资源。
        （说明：线程处于joinable状态下）

        调用该函数的线程将挂起，等待 th 所表示的线程的结束。 
        thread_return 是指向线程 th 返回值的指针。
        需要注意的是 th 所表示的线程必须是 joinable 的，
        即处于非 detached（游离）状态；
        并且只可以有唯一的一个线程对 th 调用 pthread_join() 。
        如果 th 处于 detached 状态，那么对 th 的 pthread_join() 调用将返回错误。

        如果不关心一个线程的结束状态，那么也可以将一个线程设置为 detached 状态，
        从而让操作系统在该线程结束时来回收它所占的资源。
        将一个线程设置为detached 状态可以通过两种方式来实现。
        一种是调用 pthread_detach() 函数，可以将线程 th 设置为 detached 状态。
        另一种方法是在创建线程时就将它设置为 detached 状态，
        首先初始化一个线程属性变量，然后将其设置为 detached 状态，
        最后将它作为参数传入线程创建函数 pthread_create()，
        这样所创建出来的线程就直接处于 detached 状态。

创建 detach 线程：

pthread_t tid;

pthread_attr_t attr;

pthread_attr_init(&attr);

pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

pthread_create(&tid, &attr, THREAD_FUNCTION, arg);
        总之为了在使用 pthread 时避免线程的资源在线程结束时不能得到正确释放，
        从而避免产生潜在的内存泄漏问题，在对待线程结束时，
        要确保该线程处于 detached 状态，否着就需要调用 pthread_join() 函数来对其进行资源回收。
*/
#include<stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>

#define NUM_THREADS 4

void* BusyWork(void* t)
{
    int i;
    long tid;
    double result = 0.0;
    tid = (long)t;
    printf("Thread %ld starting...\n", tid);
    for (i = 0; i < 1000000; i++)
    {
        result = result + sin(i) * tan(i);
    }
    printf("Thread %ld done. Result = %e\n", tid, result);
    pthread_exit((void*)t);
}

int main(int argc, char* argv[])
{
    pthread_t thread[NUM_THREADS];
    pthread_attr_t attr;
    int rc;
    long t;
    void* status;
    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (t = 0; t < NUM_THREADS; t++)
    {
        printf("Main: creating thread %ld\n", t);
        rc = pthread_create(&thread[t], &attr, BusyWork, (void*)t);
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    /* Free attribute and wait for the other threads */
    pthread_attr_destroy(&attr);


    for (t = 0; t < NUM_THREADS; t++)
    {
        rc = pthread_join(thread[t], &status);
        if (rc)
        {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
        printf("Main: completed join with thread %ld having a status of %ld\n", t, (long)status);
    }
    printf("Main: program completed. Exiting.\n");

    pthread_exit(NULL);
}