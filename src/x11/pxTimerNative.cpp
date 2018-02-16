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

#if __cplusplus < 201103L

#include "../pxTimer.h"

#include <stdlib.h>
#include <errno.h>

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

void pxSleepUS(uint64_t usToSleep)
{
    struct timespec res;

    res.tv_sec  = (usToSleep / 1000000UL);
    res.tv_nsec = (usToSleep * 1000UL) % 1000000000UL;

    while (true)
    {
        struct timespec remain;
        const int rv = clock_nanosleep(CLOCK_MONOTONIC, 0, &res, &remain);

        if (rv == 0)
        {
            break;
        }

        if (errno == EINTR)
        {
           res = remain;
           continue;
        }

        // Theoretically impossible case in our case :-)
        // At this point something went wrong but we cannot
        // return any error code as pxSleepMS returns void.
        abort();
    }
}

#else

void pxSleepMS(uint32_t msToSleep)
{
    pxSleepUS(msToSleep * 1000UL);
}

#endif // __cplusplus < 201103L
