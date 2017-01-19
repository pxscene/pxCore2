#include "rtThreadPoolNative.h"

#include <iostream>
#include <thread>

using namespace std;

#ifdef WIN32
#include <process.h>
#include <Windows.h>
#endif

#ifdef WIN32
void launchThread(void* threadPool)
#else
void* launchThread(void* threadPool)
#endif
{
  rtThreadPoolNative* pool = (rtThreadPoolNative*)threadPool;
  pool->startThread();
#ifndef WIN32
  return NULL;
#endif
}

rtThreadPoolNative::rtThreadPoolNative(int numberOfThreads) : 
    mNumberOfThreads(numberOfThreads), mRunning(false), mThreadTaskMutex(),
    mThreadTaskCondition(), mThreads(), mThreadTasks()
{
    initialize();
}

rtThreadPoolNative::~rtThreadPoolNative()
{
    if (mRunning)
    {
        destroy();
    }
}

bool rtThreadPoolNative::initialize()
{
    mRunning = true;
    for (int i = 0; i < mNumberOfThreads; i++)
    {
      uintptr_t threadHandle = _beginthread(launchThread, 0, this);
      mThreads.push_back((HANDLE) threadHandle );
    }
    return true;
}

void rtThreadPoolNative::destroy()
{
    mThreadTaskMutex.lock();
    mRunning = false; //mRunning is accessed by other threads
    mThreadTaskMutex.unlock();
    //broadcast to all the threads that we are shutting down
    mThreadTaskCondition.broadcast();
    for (int i = 0; i < mNumberOfThreads; i++)
    {
      WaitForSingleObject(mThreads[i], 10000);
      
        //make another attempt to broadcast to threads 
        mThreadTaskCondition.broadcast();
    }
}

void rtThreadPoolNative::startThread()
{
    rtThreadTask* threadTask = NULL;
    while(true)
    {
        mThreadTaskMutex.lock();
        while (mRunning && (mThreadTasks.empty()))
        {
            mThreadTaskCondition.wait(mThreadTaskMutex.getNativeMutexDescription());
        }
        if (!mRunning)
        {
            mThreadTaskMutex.unlock();
            return;
        }
        threadTask = mThreadTasks.front();
        mThreadTasks.pop_front();
        mThreadTaskMutex.unlock();
        
        if (threadTask != NULL)
        {
            threadTask->execute();
            delete threadTask;
            threadTask = NULL;
        }
    }
}

void rtThreadPoolNative::executeTask(rtThreadTask* threadTask)
{
    mThreadTaskMutex.lock();
    mThreadTasks.push_back(threadTask);
    mThreadTaskCondition.signal();
    mThreadTaskMutex.unlock();
}
