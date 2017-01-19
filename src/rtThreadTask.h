#ifndef RT_THREAD_TASK_H
#define RT_THREAD_TASK_H

#include "rtString.h"

class rtThreadTask
{  
public:
    rtThreadTask(void (*functionPointer)(void*), void* data, rtString key);
    ~rtThreadTask();
    void execute();
    rtString getKey();
    
private:
    void (*mFunctionPointer)(void*);
    void* mData;
    rtString mKey;
};

#endif //RT_THREAD_TASK_H