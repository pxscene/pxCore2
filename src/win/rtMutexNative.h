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

#ifndef RT_MUTEX_NATIVE_H
#define RT_MUTEX_NATIVE_H

#include <mutex>
#include <condition_variable>

typedef struct
{
  std::mutex* Mutex;
} rtMutexNativeDesc;

class rtMutexNative
{
public:
    rtMutexNative();
    ~rtMutexNative();
    void lock();
    void unlock();
    rtMutexNativeDesc getNativeMutexDescription();
private:
    std::mutex mLock;
    volatile bool mIsLocked;
};

class rtThreadConditionNative
{
public:
    rtThreadConditionNative();
    ~rtThreadConditionNative();
    void wait(rtMutexNativeDesc mutexNativeDesc);
    void signal();
    void broadcast();
private:
    std::condition_variable mCondition;
};

#endif //RT_MUTEX_NATIVE_H
