/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

// pxTimerNative.cpp

#include "../pxTimer.h"

#include <stdlib.h>

#define USE_CGT

#ifndef USE_CGT
#include <sys/time.h>
#else
#include <time.h>
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
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 * msToSleep;
    select(0, NULL, NULL, NULL, &tv);
}
