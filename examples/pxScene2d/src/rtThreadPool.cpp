#include "rtThreadPool.h"
#include "rtLog.h"
#include <iostream>
using namespace std;

#define RT_THREAD_POOL_DEFAULT_THREAD_COUNT 6

rtThreadPool* rtThreadPool::mGlobalInstance = NULL;


rtThreadPool::rtThreadPool(int numberOfThreads) : rtThreadPoolNative(numberOfThreads)
{
}

rtThreadPool::~rtThreadPool()
{
}

rtThreadPool* rtThreadPool::globalInstance()
{
    if (mGlobalInstance == NULL)
    {rtLogInfo("rtThreadPool::globalInstance");
        mGlobalInstance = new rtThreadPool(RT_THREAD_POOL_DEFAULT_THREAD_COUNT);
    }
    return mGlobalInstance;
}
