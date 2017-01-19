
#include "rtCore.h"

#ifndef RT_MUTEX_H
#define RT_MUTEX_H

class rtMutex : public rtMutexNative
{
public:
    rtMutex() {}
    ~rtMutex() {}
};

class rtThreadCondition : public rtThreadConditionNative
{
public:
    rtThreadCondition() {}
    ~rtThreadCondition() {}
};

class rtMutexLockGuard
{
public:
  rtMutexLockGuard(rtMutex& m) : m_mutex(m) { m_mutex.lock(); }
  ~rtMutexLockGuard() { m_mutex.unlock(); }
private:
  rtMutex& m_mutex;
};

#endif //RT_MUTEX_H
