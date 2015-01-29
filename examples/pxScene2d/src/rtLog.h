// rtLog.h CopyRight 2005-2006 John Robinson

#ifndef RT_LOG_H
#define RT_LOG_H

#ifdef DEBUG
#define RTLOG rtLog
#else
#define RTLOG
#endif

#ifndef RTLOGPREFIX
#define RTLOGPREFIX "rt:"
#endif

enum rtLogLevel
{
  eRT_DEBUG = 0,
  eRT_INFO  = 1,
  eRT_WARN  = 2,
  eRT_ERROR = 3,
  eRT_FATAL = 4
};

void rtLog(const char* format, ...);

void rtLog2(rtLogLevel level, const char* file, int line, const char* format, ...);

#define rtLogger(LEVEL, FORMAT, ...) do { rtLog2(eRT_ ## LEVEL, __FILE__, __LINE__, FORMAT, ## __VA_ARGS__); } while (0)
#define rtLogDebug(FORMAT, ...) rtLogger(DEBUG, FORMAT, ## __VA_ARGS__)
#define rtLogInfo(FORMAT, ...) rtLogger(INFO, FORMAT, ## __VA_ARGS__)
#define rtLogWarn(FORMAT, ...) rtLogger(WARN, FORMAT, ## __VA_ARGS__)
#define rtLogError(FORMAT, ...) rtLogger(ERROR, FORMAT, ## __VA_ARGS__)
#define rtLogFatal(FORMAT, ...) rtLogger(FATAL, FORMAT, ## __VA_ARGS__)

#endif

