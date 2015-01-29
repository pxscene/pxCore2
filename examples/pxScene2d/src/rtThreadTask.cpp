#include "rtThreadTask.h"

#include <stddef.h>

rtThreadTask::rtThreadTask(void (*functionPointer)(void*), void* data) : 
    mFunctionPointer(functionPointer), mData(data)
{
}

rtThreadTask::~rtThreadTask()
{
    mFunctionPointer = NULL;
    mData = NULL;
}

void rtThreadTask::execute()
{
    if (mFunctionPointer != NULL)
    {
        (*mFunctionPointer)(mData);
    }
}