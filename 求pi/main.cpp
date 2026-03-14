#include <iostream>
#include <pthread.h>
#include <vector>
#include <iomanip>

using namespace std;

// 全局变量
long long n = 10000000;   // 区间划分数，越大越精确
int thread_count = 4;     // 线程数量
double sum = 0.0;         // 全局求和结果
pthread_mutex_t mutexsum; // 互斥锁

// 线程函数声明
void* Thread_sum(void* rank);

int main() {
    // 初始化互斥锁
    pthread_mutex_init(&mutexsum, NULL);

    // 创建线程ID数组和线程参数数组
    vector<pthread_t> thread_ID(thread_count);
    vector<int> value(thread_count);

    // 给每个线程分配编号
    for (int i = 0; i < thread_count; i++) {
        value[i] = i;
    }

    // 创建线程
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thread_ID[i], NULL, Thread_sum, &value[i]);
    }

    // 等待线程结束
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thread_ID[i], NULL);
    }

    // 销毁互斥锁
    pthread_mutex_destroy(&mutexsum);

    // 输出结果
    cout << fixed << setprecision(15);
    cout << "Using " << thread_count << " threads, n = " << n << endl;
    cout << "Estimated pi = " << sum << endl;

    return 0;
}

// 线程函数：每个线程计算一部分积分
void* Thread_sum(void* rank) {
    int my_rank = *(int*)rank;
    double my_sum = 0.0;

    long long my_n = n / thread_count;
    long long my_first_i = my_rank * my_n;
    long long my_last_i;

    // 最后一个线程负责处理剩余部分
    if (my_rank == thread_count - 1) {
        my_last_i = n;
    }
    else {
        my_last_i = my_first_i + my_n;
    }

    for (long long i = my_first_i; i < my_last_i; i++) {
        double x = (i + 0.5) / (double)n;
        my_sum += 4.0 / (1.0 + x * x) / (double)n;
    }

    // 加锁更新全局 sum
    pthread_mutex_lock(&mutexsum);
    sum += my_sum;
    pthread_mutex_unlock(&mutexsum);

    return NULL;
}