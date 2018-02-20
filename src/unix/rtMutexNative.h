#ifndef RT_MUTEX_NATIVE_H
#define RT_MUTEX_NATIVE_H

#include <pthread.h>

typedef struct
{
    pthread_mutex_t* nativeMutexPointer;
} rtMutexNativeDesc;

class rtMutexNative
{
public:
    rtMutexNative();
    ~rtMutexNative();
    void lock();
    void unlock();
    rtMutexNativeDesc getNativeMutexDescription();
private:
    pthread_mutex_t mLock;
};

class rtThreadConditionNative
{
public:
    rtThreadConditionNative();
    ~rtThreadConditionNative();
    void wait(rtMutexNativeDesc mutexNativeDesc);
    void signal();
    void broadcast();
private:
    pthread_cond_t mCondition;
};

#endif //RT_MUTEX_NATIVE_H
