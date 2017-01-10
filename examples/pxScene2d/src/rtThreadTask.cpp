#include "rtThreadTask.h"

#include <stddef.h>

rtThreadTask::rtThreadTask(void (*functionPointer)(void*), void* data, rtString key) :
    mFunctionPointer(functionPointer), mData(data), mKey(key)
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

rtString rtThreadTask::getKey()
{
    return mKey;
}