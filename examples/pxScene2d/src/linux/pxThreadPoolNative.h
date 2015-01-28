#ifndef PX_THREAD_POOL_NATIVE_H
#define PX_THREAD_POOL_NATIVE_H

#include "../pxMutex.h"
#include "../pxThreadTask.h"

#include <pthread.h>

#include <vector>
#include <deque>

using namespace std;

class pxThreadPoolNative
{
public:
    pxThreadPoolNative(int numberOfThreads);
    ~pxThreadPoolNative();
    
    void executeTask(pxThreadTask* threadTask);
    void startThread();
    
protected:
    
    bool initialize();
    void destroy();
    
    int mNumberOfThreads;
    bool mRunning;
    pxMutex mThreadTaskMutex;
    pxThreadCondition mThreadTaskCondition;
    std::vector<pthread_t> mThreads;
    std::deque<pxThreadTask*> mThreadTasks;
};

#endif //PX_THREAD_POOL_H