// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxTimerNative.cpp

#include "pxTimer.h"

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

double  pxSeconds()
{
	UInt64 t;
	Microseconds((UnsignedWide*) &t);
	return (double)t / 1000000;
}

double pxMilliseconds()
{
	UInt64 t;
	Microseconds((UnsignedWide*) &t);
	return (double)t / 1000;
}

double  pxMicroseconds()
{
	UInt64 t;
	Microseconds((UnsignedWide*) &t);
	return t;
}

void pxSleepMS(unsigned long msToSleep)
{
	usleep(msToSleep*1000);
}
