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


rtMutexNative::rtMutexNative() : mLock(), mIsLocked(false)
{
}

rtMutexNative::~rtMutexNative()
{
    while(mIsLocked);
    unlock();
}

void rtMutexNative::lock()
{
  mLock.lock();
  mIsLocked = true;
}

void rtMutexNative::unlock()
{
  mIsLocked = false;
  mLock.unlock();
}

rtMutexNativeDesc rtMutexNative::getNativeMutexDescription()
{
    rtMutexNativeDesc desc;
    desc.Mutex = &mLock;
    return desc;
}

rtThreadConditionNative::rtThreadConditionNative() : mCondition()
{
}

rtThreadConditionNative::~rtThreadConditionNative()
{
}

void rtThreadConditionNative::wait(rtMutexNativeDesc mutexNativeDesc)
{
  std::unique_lock<std::mutex> lock(*mutexNativeDesc.Mutex);
  mCondition.wait(lock);
}

void rtThreadConditionNative::signal()
{
  mCondition.notify_one();
}

void rtThreadConditionNative::broadcast()
{
  mCondition.notify_all();
}
