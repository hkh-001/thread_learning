// Project1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>

#include <stdio.h> 
#include <pthread.h> 
#include <assert.h> 

void* Function_t(void* Param)
{
	printf("This is pthread！ ");
	pthread_t myid = pthread_self();
	printf("threadID=%d ", myid);
	return NULL;
}

int main()
{
	pthread_t pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&pid, &attr, Function_t, NULL);
	printf("-----");
	getchar();
	pthread_attr_destroy(&attr);
	return 1;
}
