#ifndef PX_MUTEX_NATIVE_H
#define PX_MUTEX_NATIVE_H

#include <pthread.h>

typedef struct
{
    pthread_mutex_t* nativeMutexPointer;
} pxMutexNativeDesc;

class pxMutexNative
{
public:
    pxMutexNative();
    ~pxMutexNative();
    void lock();
    void unlock();
    pxMutexNativeDesc getNativeMutexDescription();
private:
    pthread_mutex_t mLock;
    volatile bool mIsLocked;
};

class pxThreadConditionNative
{
public:
    pxThreadConditionNative();
    ~pxThreadConditionNative();
    void wait(pxMutexNativeDesc mutexNativeDesc);
    void signal();
    void broadcast();
private:
    pthread_cond_t mCondition;
};

#endif //PX_MUTEX_NATIVE_H