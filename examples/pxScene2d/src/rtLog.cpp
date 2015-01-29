// rtLog.cpp CopyRight 2005-2015 John Robinson
//#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
//#include "windows.h"
#include <stdarg.h>
#include <string.h>
#include "rtLog.h"
#include <unistd.h>
#include <sys/syscall.h>

void rtLog(const char* format, ...)
{
  // JRXXX
#if 0
    char buffer[1024];
    buffer[0] = 0;
    va_list ptr; va_start(ptr, format);
    strcpy(buffer, RTLOGPREFIX);
    if (_vsnprintf(buffer+strlen(RTLOGPREFIX), sizeof(buffer)-strlen(buffer)-1, format, ptr) < 1)
    {
        strcpy(buffer, "rtLog buffer overflow\n");
    }
    OutputDebugStringA(buffer);
    va_end(ptr);
#else
    char buffer[1024];
    buffer[0] = 0;
    va_list ptr; va_start(ptr, format);
    strcpy(buffer, RTLOGPREFIX);
    if (vsnprintf(buffer+strlen(RTLOGPREFIX), sizeof(buffer)-strlen(buffer)-1, format, ptr) < 1)
    {
        strcpy(buffer, "rtLog buffer overflow\n");
    }
    //    OutputDebugStringA(buffer);
    printf("%s", buffer);
    va_end(ptr);
#endif
}

namespace
{
  const char* rtLogLevelStrings[] = 
  {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
  };

  const char* rtLogLevelToString(rtLogLevel l)
  {
    const char* s = "OUT-OF-BOUNDS";
    if (l < sizeof(rtLogLevelStrings))
      s = rtLogLevelStrings[l];
    return s;
  }

  const char* rtTrimPath(const char* s)
  {
    if (!s)
      return s;

    const char* t = strrchr(s, (int) '/');
    if (t) t++;

    return t;
  }
}

void rtLog2(rtLogLevel level, const char* file, int line, const char* format, ...)
{
  printf("rt:%5s %s:%d -- Thread-%lu: ", rtLogLevelToString(level), rtTrimPath(file), line, syscall(__NR_gettid));

  va_list ptr;
  va_start(ptr, format);
  vprintf(format, ptr);
  va_end(ptr);

  printf("\n");
  
  if (level == RT_LOG_FATAL)
  {
    abort();
  }
}

