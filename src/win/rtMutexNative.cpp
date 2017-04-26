#include "rtMutexNative.h"


rtMutexNative::rtMutexNative() : mLock(), mIsLocked(false)
{
}

rtMutexNative::~rtMutexNative()
{
    while(mIsLocked);
    unlock();
}

void rtMutexNative::lock()
{
  mLock.lock();
  mIsLocked = true;
}

void rtMutexNative::unlock()
{
  mIsLocked = false;
  mLock.unlock();
}

rtMutexNativeDesc rtMutexNative::getNativeMutexDescription()
{
    rtMutexNativeDesc desc;
    desc.Mutex = &mLock;
    return desc;
}

rtThreadConditionNative::rtThreadConditionNative() : mCondition()
{
}

rtThreadConditionNative::~rtThreadConditionNative()
{
}

void rtThreadConditionNative::wait(rtMutexNativeDesc mutexNativeDesc)
{
	std::unique_lock<std::mutex> lock(*mutexNativeDesc.Mutex);
	mCondition.wait(lock);
}

void rtThreadConditionNative::signal()
{
  mCondition.notify_one();
}

void rtThreadConditionNative::broadcast()
{
  mCondition.notify_all();
}