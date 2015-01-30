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

struct LogLevelSetter
{
  LogLevelSetter()
  {
    const char* s = getenv("RT_LOG_LEVEL");
    if (s)
    {
      rtLogLevel level = RT_LOG_ERROR;
      if      (strcasecmp(s, "debug") == 0) level = RT_LOG_DEBUG;
      else if (strcasecmp(s, "info") == 0)  level = RT_LOG_INFO;
      else if (strcasecmp(s, "warn") == 0)  level = RT_LOG_WARN;
      else if (strcasecmp(s, "error") == 0) level = RT_LOG_ERROR;
      else if (strcasecmp(s, "fatal") == 0) level = RT_LOG_FATAL;
      else
      {
        fprintf(stderr, "invalid RT_LOG_LEVEL set: %s", s);
        abort();
      }
      rtLog2SetLevel(level);
    }
  }
};

static LogLevelSetter __logLevelSetter; // force RT_LOG_LEVEL to be read from env

static const char* rtLogLevelStrings[] =
{
  "DEBUG",
  "INFO",
  "WARN",
  "ERROR",
  "FATAL"
};

static const char* rtLogLevelToString(rtLogLevel l)
{
  const char* s = "OUT-OF-BOUNDS";
  if (l < sizeof(rtLogLevelStrings))
    s = rtLogLevelStrings[l];
  return s;
}

static const char* rtTrimPath(const char* s)
{
  if (!s)
    return s;

  const char* t = strrchr(s, (int) '/');
  if (t) t++;

  return t;
}

static rtLogHandler sLogHandler = NULL;
void rtLog2SetLogHandler(rtLogHandler logHandler)
{
  sLogHandler = logHandler;
}

static rtLogLevel sLevel = RT_LOG_INFO;
void rtLog2SetLevel(rtLogLevel level)
{
  sLevel = level;
}

void rtLog2(rtLogLevel level, const char* file, int line, const char* format, ...)
{
  if (level < sLevel)
    return;

  const char* logLevel = rtLogLevelToString(level);
  const char* path = rtTrimPath(file);
  const int   threadId = syscall(__NR_gettid);

  if (sLogHandler == NULL)
  {
    printf(RTLOGPREFIX "%5s %s:%d -- Thread-%d: ", logLevel, path, line, threadId);

    va_list ptr;
    va_start(ptr, format);
    vprintf(format, ptr);
    va_end(ptr);

    printf("\n");
  }
  else
  {
    size_t n = 0;
    char buff[1024];
    va_list args;

    va_start(args, format);
    n = vsnprintf(buff, sizeof(buff), format, args);
    va_end(args);

    if (n >= sizeof(buff))
      buff[sizeof(buff) - 1] = '\0';

    sLogHandler(level, path, line, threadId, buff);
  }
  
  if (level == RT_LOG_FATAL)
    abort();
}

