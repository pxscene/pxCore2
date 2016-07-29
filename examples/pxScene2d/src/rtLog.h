// rtCore Copyright 2005-2015 John Robinson
// rtLog.h

#ifndef RT_LOG_H
#define RT_LOG_H

#include <stdlib.h>

#ifdef DEBUG
#define RTLOG rtLog
#else
#define RTLOG
#endif

#ifndef RTLOGPREFIX
#define RTLOGPREFIX "rt:"
#endif

#ifdef __GNUC__
#define RT_PRINTF_FORMAT(IDX, FIRST) __attribute__ ((format (printf, IDX, FIRST)))
#else
#define RT_PRINTF_FORMAT(IDX, FIRST)
#endif


enum rtLogLevel
{
  RT_LOG_DEBUG = 0,
  RT_LOG_INFO  = 1,
  RT_LOG_WARN  = 2,
  RT_LOG_ERROR = 3,
  RT_LOG_FATAL = 4
};

typedef void (*rtLogHandler)(rtLogLevel level, const char* file, int line, int threadId, char* message);

void rtLogPrintf(rtLogLevel level, const char* file, int line, const char* format, ...) RT_PRINTF_FORMAT(4, 5);
void rtLogSetLevel(rtLogLevel l);
void rtLogSetLogHandler(rtLogHandler logHandler);

class rtLogEnterExit
{
public:
  rtLogEnterExit(char const* file, char const* func, int line)
    : m_file(file)
    , m_func(func)
    , m_line(line)
  {
    if (s_enabled == 0)
    {
      if (getenv("RT_TRACE_FUNCTIONS"))
        s_enabled = 2;
      else
        s_enabled = 1;
    }

    if (s_enabled == 2)
      rtLogPrintf(RT_LOG_INFO, m_file, m_line, "rt_debug_enter (%s)", m_func);
  }

  ~rtLogEnterExit()
  {
    if (s_enabled == 2)
      rtLogPrintf(RT_LOG_INFO, m_file, m_line, "rt_debug_exit  (%s)", m_func);
  }

private:
  static int  s_enabled;
  char const* m_file;
  char const* m_func;
  int         m_line;
};

#ifdef RT_DEBUG
#define rtLogEnter() rtLogEnterExit __enter_exit__(__FILE__, __PRETTY_FUNCTION__, __LINE__)
#else
#define rtLogEnter()
#endif

#define rtLog(LEVEL, FORMAT, ...) do { rtLogPrintf(LEVEL, __FILE__, __LINE__, FORMAT, ## __VA_ARGS__); } while (0)
#define rtLogDebug(FORMAT, ...) rtLog(RT_LOG_DEBUG, FORMAT, ## __VA_ARGS__)
#define rtLogInfo(FORMAT, ...) rtLog(RT_LOG_INFO, FORMAT, ## __VA_ARGS__)
#define rtLogWarn(FORMAT, ...) rtLog(RT_LOG_WARN, FORMAT, ## __VA_ARGS__)
#define rtLogError(FORMAT, ...) rtLog(RT_LOG_ERROR, FORMAT, ## __VA_ARGS__)
#define rtLogFatal(FORMAT, ...) rtLog(RT_LOG_FATAL, FORMAT, ## __VA_ARGS__)

#endif

