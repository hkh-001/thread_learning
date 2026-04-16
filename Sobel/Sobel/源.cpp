#include<stdio.h>
#include<pthread.h>
#include<Windows.h>

void* Function_t(void* Param)
{
	pthread_t myid = pthread_self();
	while (1)
	{
		printf("线程ID=%d \n", myid);
		Sleep(1000);
	}
	return NULL;
}

int main()
{
	pthread_t pid;
	pthread_create(&pid, NULL, Function_t, NULL);
	while (1)
	{
		printf("in fatherprocess!\n");
		Sleep(1000);
	}
	getchar();
	return 1;
}
