#include "pxThreadPool.h"

#include <iostream>
using namespace std;

#define PX_THREAD_POOL_DEFAULT_THREAD_COUNT 6

pxThreadPool* pxThreadPool::mGlobalInstance = NULL;


pxThreadPool::pxThreadPool(int numberOfThreads) : pxThreadPoolNative(numberOfThreads)
{
}

pxThreadPool::~pxThreadPool()
{
}

pxThreadPool* pxThreadPool::globalInstance()
{
    if (mGlobalInstance == NULL)
    {
        mGlobalInstance = new pxThreadPool(PX_THREAD_POOL_DEFAULT_THREAD_COUNT);
    }
    return mGlobalInstance;
}