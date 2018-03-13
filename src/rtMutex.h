
/*

 rtCore Copyright 2005-2018 John Robinson

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

// rtMutex.h

#include "rtCore.h"

#ifndef RT_MUTEX_H
#define RT_MUTEX_H

class rtMutex : public rtMutexNative
{
public:
    rtMutex() {}
    ~rtMutex() {}
};

class rtThreadCondition : public rtThreadConditionNative
{
public:
    rtThreadCondition() {}
    ~rtThreadCondition() {}
};

class rtMutexLockGuard
{
public:
  rtMutexLockGuard(rtMutex& m) : m_mutex(m) { m_mutex.lock(); }
  ~rtMutexLockGuard() { m_mutex.unlock(); }
private:
  rtMutex& m_mutex;
};

#endif //RT_MUTEX_H
