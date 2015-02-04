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
  RT_LOG_DEBUG = 0,
  RT_LOG_INFO  = 1,
  RT_LOG_WARN  = 2,
  RT_LOG_ERROR = 3,
  RT_LOG_FATAL = 4
};

typedef void (*rtLogHandler)(rtLogLevel level, const char* file, int line, int threadId, char* message);

void rtLogPrintf(rtLogLevel level, const char* file, int line, const char* format, ...);
void rtLogSetLevel(rtLogLevel l);
void rtLogSetLogHandler(rtLogHandler logHandler);

#define rtLog(LEVEL, FORMAT, ...) do { rtLogPrintf(LEVEL, __FILE__, __LINE__, FORMAT, ## __VA_ARGS__); } while (0)
#define rtLogDebug(FORMAT, ...) rtLog(RT_LOG_DEBUG, FORMAT, ## __VA_ARGS__)
#define rtLogInfo(FORMAT, ...) rtLog(RT_LOG_INFO, FORMAT, ## __VA_ARGS__)
#define rtLogWarn(FORMAT, ...) rtLog(RT_LOG_WARN, FORMAT, ## __VA_ARGS__)
#define rtLogError(FORMAT, ...) rtLog(RT_LOG_ERROR, FORMAT, ## __VA_ARGS__)
#define rtLogFatal(FORMAT, ...) rtLog(RT_LOG_FATAL, FORMAT, ## __VA_ARGS__)

#endif

