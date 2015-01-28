#include "pxMutexNative.h"


pxMutexNative::pxMutexNative() : mLock(), mIsLocked(false)
{
    pthread_mutex_init(&mLock, NULL);
}

pxMutexNative::~pxMutexNative()
{
    while(mIsLocked);
    unlock();
    pthread_mutex_destroy(&mLock);
}

void pxMutexNative::lock()
{
    pthread_mutex_lock(&mLock);
    mIsLocked = true;
}

void pxMutexNative::unlock()
{
    mIsLocked = false;
    pthread_mutex_unlock(&mLock);
}

pxMutexNativeDesc pxMutexNative::getNativeMutexDescription()
{
    pxMutexNativeDesc desc;
    desc.nativeMutexPointer = &mLock;
    return desc;
}

pxThreadConditionNative::pxThreadConditionNative() : mCondition()
{
    pthread_cond_init(&mCondition, NULL);
}

pxThreadConditionNative::~pxThreadConditionNative()
{
    pthread_cond_destroy(&mCondition);
}

void pxThreadConditionNative::wait(pxMutexNativeDesc mutexNativeDesc)
{
    pthread_cond_wait(&mCondition, mutexNativeDesc.nativeMutexPointer);
}

void pxThreadConditionNative::signal()
{
    pthread_cond_signal(&mCondition);
}

void pxThreadConditionNative::broadcast()
{
    pthread_cond_broadcast(&mCondition);
}