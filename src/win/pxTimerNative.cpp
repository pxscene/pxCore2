// pxTimerNative.cpp

#include <windows.h>
#include <stdint.h>

static bool gFreqInitialized = false;
static LARGE_INTEGER gFreq;

double pxSeconds()
{
    if (!gFreqInitialized)
    {
        ::QueryPerformanceFrequency(&gFreq);
        gFreqInitialized = true;
    }

    LARGE_INTEGER c;
    ::QueryPerformanceCounter(&c);

    return (c.QuadPart / (double)gFreq.QuadPart);
}

double pxMilliseconds()
{
    if (!gFreqInitialized)
    {
        ::QueryPerformanceFrequency(&gFreq);
        gFreqInitialized = true;
    }

    LARGE_INTEGER c;
    ::QueryPerformanceCounter(&c);

    return (c.QuadPart * 1000) / (double)gFreq.QuadPart;
}

double pxMicroseconds()
{
    if (!gFreqInitialized)
    {
        ::QueryPerformanceFrequency(&gFreq);
        gFreqInitialized = true;
    }

    LARGE_INTEGER c;
    ::QueryPerformanceCounter(&c);

    return (c.QuadPart * 1000000) / (double)gFreq.QuadPart;
}

void pxSleepMS(uint32_t sleepMS)
{
    Sleep(sleepMS);
}
