/*
互斥锁 Mutex
        Mutex 常常被用来保护那些可以被多个线程访问的共享资源，
        比如可以防止多个线程同时更新同一个数据时出现混乱。

使用互斥锁的一般步骤是：

创建一个互斥锁，即声明一个pthread_mutex_t类型的数据，然后初始化，只有初始化之后才能使用；
多个线程尝试锁定这个互斥锁；
只有一个成功锁定互斥锁，成为互斥锁的拥有者，然后进行一些指令；
拥有者解锁互斥锁；
其他线程尝试锁定这个互斥锁，重复上面的过程；
最后互斥锁被显式地调用 pthread_mutex_destroy 来进行销毁。
有两种方式初始化一个互斥锁：

第一种，利用已经定义的常量初始化，例如

       pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

第二种方式是调用 pthread_mutex_init (mutex,attr) 进行初始化。

        当多个线程同时去锁定同一个互斥锁时，失败的那些线程，如果是用 pthread_mutex_lock 函数，
        那么会被阻塞，直到这个互斥锁被解锁，它们再继续竞争；如果是用 pthread_mutex_trylock 函数，
        那么失败者只会返回一个错误。

        最后需要指出的是，保护共享数据是程序员的责任。
        程序员要负责所有可以访问该数据的线程都使用mutex这种机制，
        否则，不使用 mutex 的线程还是有可能对数据造成破坏。

相关函数 (具体声明可以用 man 查看 )

pthread_mutex_init (mutex,attr);
pthread_mutex_destroy (pthread_mutex_t *mutex);
pthread_mutexattr_init (attr);
pthread_mutexattr_destroy (attr);
phtread_mutex_lock(pthread_mutex_t *mutex);
phtread_mutex_trylock(pthread_mutex_t *mutex);
phtread_mutex_unlock(pthread_mutex_t *mutex);

*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*
The following structure contains the necessary information
to allow the function "dotprod" to access its input data and
place its output into the structure.  This structure is
unchanged from the sequential version.
*/

typedef struct
{
    double* a;
    double* b;
    double sum;
    int veclen;
} DOTDATA;

/* Define globally accessible variables and a mutex */

#define NUMTHRDS 4
#define VECLEN 100000
DOTDATA dotstr;
pthread_t callThd[NUMTHRDS];
pthread_mutex_t mutexsum;

/*
The function dotprod is activated when the thread is created.
As before, all input to this routine is obtained from a structure
of type DOTDATA and all output from this function is written into
this structure. The benefit of this approach is apparent for the
multi-threaded program: when a thread is created we pass a single
argument to the activated function - typically this argument
is a thread number. All  the other information required by the
function is accessed from the globally accessible structure.
*/

void* dotprod(void* arg)
{

    /* Define and use local variables for convenience */

    int i, start, end, len;
    long offset;
    double mysum, * x, * y;
    offset = (long)arg;

    len = dotstr.veclen;
    start = offset * len;
    end = start + len;
    x = dotstr.a;
    y = dotstr.b;

    /*
    Perform the dot product and assign result
    to the appropriate variable in the structure.
    */
    mysum = 0;
    for (i = start; i < end; i++)
    {
        mysum += (x[i] * y[i]);
    }

    /*
    Lock a mutex prior to updating the value in the shared
    structure, and unlock it upon updating.
    */
    pthread_mutex_lock(&mutexsum);
    dotstr.sum += mysum;
    printf("Thread %ld did %d to %d: mysum=%f global sum=%f\n", offset, start,
        end, mysum, dotstr.sum);
    pthread_mutex_unlock(&mutexsum);
    pthread_exit((void*)0);
}

/*
The main program creates threads which do all the work and then print out result
upon completion. Before creating the threads, The input data is created. Since
all threads update a shared structure, we need a mutex for mutual exclusion.
The main thread needs to wait for all threads to complete, it waits for each one
of the threads. We specify a thread attribute value that allow the main thread to
join with the threads it creates. Note also that we free up handles  when they
are no longer needed.
*/

int main(int argc, char* argv[])
{
    long i;
    double* a, * b;
    void* status;
    pthread_attr_t attr;

    /* Assign storage and initialize values */

    a = (double*)malloc(NUMTHRDS * VECLEN * sizeof(double));
    b = (double*)malloc(NUMTHRDS * VECLEN * sizeof(double));

    for (i = 0; i < VECLEN * NUMTHRDS; i++)
    {
        a[i] = 1;
        b[i] = a[i];
    }

    dotstr.veclen = VECLEN;
    dotstr.a = a;
    dotstr.b = b;
    dotstr.sum = 0;

    pthread_mutex_init(&mutexsum, NULL);

    /* Create threads to perform the dotproduct  */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (i = 0; i < NUMTHRDS; i++)
    {
        /* Each thread works on a different set of data.
         * The offset is specified by 'i'. The size of
         * the data for each thread is indicated by VECLEN.
         */
        pthread_create(&callThd[i], &attr, dotprod, (void*)i);
    }

    pthread_attr_destroy(&attr);
    /* Wait on the other threads */

    for (i = 0; i < NUMTHRDS; i++)
    {
        pthread_join(callThd[i], &status);
    }
    /* After joining, print out the results and cleanup */

    printf("Sum =  %f \n", dotstr.sum);
    free(a);
    free(b);
    pthread_mutex_destroy(&mutexsum);
    pthread_exit(NULL);
}