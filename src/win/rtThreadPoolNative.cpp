/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

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
			mThreadTaskMutex.unlock();
            mThreadTaskCondition.wait(mThreadTaskMutex.getNativeMutexDescription());
			mThreadTaskMutex.lock();
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
