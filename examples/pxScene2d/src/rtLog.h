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

void rtLog(const char* format, ...);

#endif
