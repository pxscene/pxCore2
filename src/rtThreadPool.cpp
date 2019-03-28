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
#include <string.h>
#include <stdlib.h>
using namespace std;

#define RT_THREAD_POOL_DEFAULT_THREAD_COUNT 6

int numberOfDefaultThreads()
{
  static int numberOfThreads = RT_THREAD_POOL_DEFAULT_THREAD_COUNT;
  static bool once = true;
  if (once)
  {
    char const *s = getenv("RT_THREAD_POOL_SIZE");
    if (s)
    {
      if (strlen(s) > 0)
      {
        numberOfThreads = atoi(s);
      }
    }
    if (numberOfThreads <= 0)
    {
      printf("The number of threads in the rt thread pool is too small: %d.  Defaulting to %d\n", numberOfThreads,
                RT_THREAD_POOL_DEFAULT_THREAD_COUNT);
      numberOfThreads = RT_THREAD_POOL_DEFAULT_THREAD_COUNT;
    }
  }
  printf("The default number of threads in the rt thread pool is %d\n", numberOfThreads);
  return numberOfThreads;
}

rtThreadPool* rtThreadPool::mGlobalInstance = new rtThreadPool(numberOfDefaultThreads());


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
        mGlobalInstance = new rtThreadPool(numberOfDefaultThreads());
    }
    return mGlobalInstance;
}

int rtThreadPool::numberOfThreadsInPool()
{
  return mNumberOfThreads;
}
