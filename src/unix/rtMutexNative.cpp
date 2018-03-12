#include "rtMutexNative.h"


rtMutexNative::rtMutexNative() : mLock(), mIsLocked(false)
{
    pthread_mutex_init(&mLock, NULL);
}

rtMutexNative::~rtMutexNative()
{
    while(mIsLocked);
    pthread_mutex_destroy(&mLock);
}

void rtMutexNative::lock()
{
    pthread_mutex_lock(&mLock);
    mIsLocked = true;
}

void rtMutexNative::unlock()
{
    pthread_mutex_unlock(&mLock);
    mIsLocked = false;
}

rtMutexNativeDesc rtMutexNative::getNativeMutexDescription()
{
    rtMutexNativeDesc desc;
    desc.nativeMutexPointer = &mLock;
    return desc;
}

rtThreadConditionNative::rtThreadConditionNative() : mCondition()
{
    pthread_cond_init(&mCondition, NULL);
}

rtThreadConditionNative::~rtThreadConditionNative()
{
    pthread_cond_destroy(&mCondition);
}

void rtThreadConditionNative::wait(rtMutexNativeDesc mutexNativeDesc)
{
    pthread_cond_wait(&mCondition, mutexNativeDesc.nativeMutexPointer);
}

void rtThreadConditionNative::signal()
{
    pthread_cond_signal(&mCondition);
}

void rtThreadConditionNative::broadcast()
{
    pthread_cond_broadcast(&mCondition);
}
