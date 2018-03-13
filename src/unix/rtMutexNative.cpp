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

#include "rtMutexNative.h"


rtMutexNative::rtMutexNative() : mLock()
{
    pthread_mutex_init(&mLock, NULL);
}

rtMutexNative::~rtMutexNative()
{
    pthread_mutex_lock(&mLock);
    pthread_mutex_unlock(&mLock);
    pthread_mutex_destroy(&mLock);
}

void rtMutexNative::lock()
{
    pthread_mutex_lock(&mLock);
}

void rtMutexNative::unlock()
{
    pthread_mutex_unlock(&mLock);
}

rtMutexNativeDesc rtMutexNative::getNativeMutexDescription()
{
    rtMutexNativeDesc desc;
    desc.nativeMutexPointer = &mLock;
    return desc;
}

rtThreadConditionNative::rtThreadConditionNative() : mCondition()
{
    pthread_cond_init(&mCondition, NULL);
}

rtThreadConditionNative::~rtThreadConditionNative()
{
    pthread_cond_destroy(&mCondition);
}

void rtThreadConditionNative::wait(rtMutexNativeDesc mutexNativeDesc)
{
    pthread_cond_wait(&mCondition, mutexNativeDesc.nativeMutexPointer);
}

void rtThreadConditionNative::signal()
{
    pthread_cond_signal(&mCondition);
}

void rtThreadConditionNative::broadcast()
{
    pthread_cond_broadcast(&mCondition);
}
