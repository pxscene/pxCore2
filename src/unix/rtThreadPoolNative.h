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
    void destroy();
    
protected:
    
    bool initialize();

    
    int mNumberOfThreads;
    bool mRunning;
    rtMutex mThreadTaskMutex;
    rtThreadCondition mThreadTaskCondition;
    std::vector<pthread_t> mThreads;
    std::deque<rtThreadTask*> mThreadTasks;
};

#endif //RT_THREAD_POOL_H