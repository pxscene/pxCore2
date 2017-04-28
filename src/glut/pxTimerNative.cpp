// pxCore CopyRight 2005-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxTimerNative.cpp

#include "../pxTimer.h"

#include "pxConfigNative.h"

#include <stdlib.h>

#ifndef USE_CGT
#include <sys/time.h>
#else
#include <time.h>
#endif

#ifdef USE_SELECT_FOR_SLEEP
#include <sys/select.h>
#else
#include <unistd.h>
#endif

double  pxSeconds()
{
#ifndef USE_CGT
  timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + ((double)tv.tv_usec/1000000);
#else
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ((double)ts.tv_nsec/1000000000);
#endif
}

double pxMilliseconds()
{
#ifndef USE_CGT
  timeval tv;
  gettimeofday(&tv, NULL);
  return ((double)(tv.tv_sec * 1000) + ((double)tv.tv_usec/1000));
#else
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ((double)(ts.tv_sec * 1000) + ((double)ts.tv_nsec/1000000));
#endif
}

double  pxMicroseconds()
{
#ifndef USE_CGT
  timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000000) + tv.tv_usec;
#else
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ((double)(ts.tv_sec * 1000000) + ((double)ts.tv_nsec/1000));
#endif
}

void pxSleepMS(uint32_t msToSleep)
{
#ifdef USE_SELECT_FOR_SLEEP
  timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 1000 * msToSleep;
  select(0, NULL, NULL, NULL, &tv);
#else
  useconds_t s = (useconds_t)msToSleep*1000;
  usleep(s);
#endif
}
