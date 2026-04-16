#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <iomanip>
#include <time.h>

#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif /* _UNISTD_H */

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#ifdef WIN32

int gettimeofday(struct timeval* tp, void* tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}
#endif

using namespace std;

int main()
{
    struct timeval time1, time2;
	double ans = 0;
    int n = 10000000000;
    gettimeofday(&time1, NULL);
	for(double i = 0; i < n; i ++)
		ans += (4/(1 + ((i + 0.5)/n)*((i + 0.5)/n)))/n;
	cout << setprecision(20) << ans << endl;
    gettimeofday(&time2, NULL);
    printf("s: %ld, ms: %ld\n", time2.tv_sec-time1.tv_sec, (time2.tv_sec*1000 + time2.tv_sec/1000)-(time1.tv_sec*1000 + time1.tv_sec/1000));
}