#ifndef RT_MUTEX_H
#define RT_MUTEX_H

#include "rtCore.h"

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

#endif //RT_MUTEX_H
