#include "rtThreadPoolNative.h"

#include <iostream>
using namespace std;

void* launchThread(void* threadPool)
{
    rtThreadPoolNative* pool = (rtThreadPoolNative*) threadPool;
    pool->startThread();
    return NULL;
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
        pthread_t tid;
        int returnValue = pthread_create(&tid, NULL, launchThread, (void*) this);
        if (returnValue != 0)
        {
            cout << "Error creating thread.  The return value is " << returnValue << endl;
            return false;
        }
        mThreads.push_back(tid);
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
        void* result;
        int returnValue = pthread_join(mThreads[i], &result);
        if (returnValue != 0)
        {
            cout << "Error joining threads" << endl;
        }
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
            pthread_exit(NULL);
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
