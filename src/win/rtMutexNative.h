#ifndef RT_MUTEX_NATIVE_H
#define RT_MUTEX_NATIVE_H

#include <mutex>
#include <condition_variable>

typedef struct
{
  std::mutex* Mutex;
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
    std::mutex mLock;
    volatile bool mIsLocked;
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
    std::condition_variable mCondition;
};

#endif //RT_MUTEX_NATIVE_H
