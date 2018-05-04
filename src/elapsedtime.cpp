

#ifdef WIN32
#include <windows.h>

LARGE_INTEGER f, t1, t2; 

void _mark ()
{
	QueryPerformanceFrequency(&f);
	QueryPerformanceCounter(&t1);
}

double _duration ()
{
	QueryPerformanceCounter(&t2);
	double elapsedTime = ((t2.QuadPart - t1.QuadPart) * 1000.0 / f.QuadPart)/1000;
	return elapsedTime;
}
#else
#include <time.h>
timespec s,f;
timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

void _mark ()
{
	clock_gettime(CLOCK_MONOTONIC,&s);
}

double _duration ()
{
	clock_gettime(CLOCK_MONOTONIC,&f);
	timespec d = diff(s,f);
	return (double) d.tv_sec + (double) d.tv_nsec/ (double) 1E9;
}
#endif
