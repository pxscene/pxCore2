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

// rtThreadPool.h

#include "rtThreadPool.h"

#include <iostream>
using namespace std;

#define RT_THREAD_POOL_DEFAULT_THREAD_COUNT 6

rtThreadPool* rtThreadPool::mGlobalInstance = new rtThreadPool(RT_THREAD_POOL_DEFAULT_THREAD_COUNT);


rtThreadPool::rtThreadPool(int numberOfThreads) : rtThreadPoolNative(numberOfThreads)
{
}

rtThreadPool::~rtThreadPool()
{
  if (mGlobalInstance == this)
  {
    mGlobalInstance = NULL;
  }
}

rtThreadPool* rtThreadPool::globalInstance()
{
    if (mGlobalInstance == NULL)
    {
        mGlobalInstance = new rtThreadPool(RT_THREAD_POOL_DEFAULT_THREAD_COUNT);
    }
    return mGlobalInstance;
}
