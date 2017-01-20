#include "rtThreadPool.h"

#include <iostream>
using namespace std;

#define RT_THREAD_POOL_DEFAULT_THREAD_COUNT 6

rtThreadPool* rtThreadPool::mGlobalInstance = new rtThreadPool(RT_THREAD_POOL_DEFAULT_THREAD_COUNT);


rtThreadPool::rtThreadPool(int numberOfThreads) : rtThreadPoolNative(numberOfThreads)
{
}

rtThreadPool::~rtThreadPool()
{
  delete mGlobalInstance;
  mGlobalInstance = NULL;
}

rtThreadPool* rtThreadPool::globalInstance()
{
    if (mGlobalInstance == NULL)
    {
        mGlobalInstance = new rtThreadPool(RT_THREAD_POOL_DEFAULT_THREAD_COUNT);
    }
    return mGlobalInstance;
}
