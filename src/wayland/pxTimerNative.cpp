// pxCore CopyRight 2005-2006 John Robinson
// Portable Framebuffer and Windowing Library
// pxTimerNative.cpp

#include "../pxTimer.h"

#include <stdlib.h>
#include <sys/time.h>

double  pxSeconds()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

double pxMilliseconds()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return ((double)(tv.tv_sec * 1000) + ((double)tv.tv_usec/1000));
}

double  pxMicroseconds()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000) + tv.tv_usec;
}

void pxSleepMS(unsigned long msToSleep)
{
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 * msToSleep;
    select(0, NULL, NULL, NULL, &tv);
}
