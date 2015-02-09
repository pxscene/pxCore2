#include "pxTimer.h"

#include <stdlib.h>
#include <sys/time.h>

double  pxSeconds()
{
  return pxMilliseconds() * 1000;
}

double pxMilliseconds()
{
  return pxMicroseconds() * 1000;
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
