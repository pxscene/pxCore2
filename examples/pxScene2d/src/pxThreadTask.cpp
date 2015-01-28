#include "pxThreadTask.h"

#include <stddef.h>

pxThreadTask::pxThreadTask(void (*functionPointer)(void*), void* data) : 
    mFunctionPointer(functionPointer), mData(data)
{
}

pxThreadTask::~pxThreadTask()
{
    mFunctionPointer = NULL;
    mData = NULL;
}

void pxThreadTask::execute()
{
    if (mFunctionPointer != NULL)
    {
        (*mFunctionPointer)(mData);
    }
}