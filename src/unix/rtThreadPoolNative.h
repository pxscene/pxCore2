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

#ifndef RT_THREAD_POOL_NATIVE_H
#define RT_THREAD_POOL_NATIVE_H

#include "../rtMutex.h"
#include "../rtThreadTask.h"
#include "../rtString.h"

#include <pthread.h>

#include <vector>
#include <deque>

class rtThreadPoolNative
{
public:
    rtThreadPoolNative(int numberOfThreads);
    ~rtThreadPoolNative();
    
    void executeTask(rtThreadTask* threadTask);
    void raisePriority(rtString key);
    void startThread();
    
protected:
    
    bool initialize();
    void destroy();
    
    int mNumberOfThreads;
    bool mRunning;
    rtMutex mThreadTaskMutex;
    rtThreadCondition mThreadTaskCondition;
    std::vector<pthread_t> mThreads;
    std::deque<rtThreadTask*> mThreadTasks;
};

#endif //RT_THREAD_POOL_H
