#ifndef RT_THREAD_POOL_H
#define RT_THREAD_POOL_H

#include "rtCore.h"

class rtThreadPool : public rtThreadPoolNative
{
public:
    rtThreadPool(int numberOfThreads);
    ~rtThreadPool();
    
    static rtThreadPool* globalInstance();

	void raisePriority(const rtString & /*url*/) {
		//TODO
	};
    
private:
    
    static rtThreadPool* mGlobalInstance;
};

#endif //RT_THREAD_POOL_H
