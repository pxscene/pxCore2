#ifndef PX_THREAD_POOL_H
#define PX_THREAD_POOL_H

#include "pxCore.h"

class pxThreadPool : public pxThreadPoolNative
{
public:
    pxThreadPool(int numberOfThreads);
    ~pxThreadPool();
    
    static pxThreadPool* globalInstance();
    
private:
    
    static pxThreadPool* mGlobalInstance;
};

#endif //PX_THREAD_POOL_H