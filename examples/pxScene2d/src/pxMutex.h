#ifndef PX_MUTEX_H
#define PX_MUTEX_H

#include "pxCore.h"

class pxMutex : public pxMutexNative
{
public:
    pxMutex() {}
    ~pxMutex() {}
};

class pxThreadCondition : public pxThreadConditionNative
{
public:
    pxThreadCondition() {}
    ~pxThreadCondition() {}
};

#endif //PX_MUTEX_H