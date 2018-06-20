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

#include <sstream>

#define private public
#define protected public

#include "rtThreadPool.h"
#include "rtString.h"
#include <string.h>

#include "test_includes.h" // Needs to be included last

using namespace std;

class rtThreadPoolTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void creationTest()
    {
      rtThreadPool p(0);
      EXPECT_TRUE(p.mNumberOfThreads == 0);
      EXPECT_TRUE(p.mRunning == true);
    }

    void destructionNonGlobalTest()
    {
      rtThreadPool *p = new rtThreadPool(0);
      delete p;
      EXPECT_TRUE(rtThreadPool::mGlobalInstance != NULL);
    }

    void destructionGlobalTest()
    {
      rtThreadPool *globInst = rtThreadPool::mGlobalInstance;
      rtThreadPool::mGlobalInstance = NULL;
      rtThreadPool *p = rtThreadPool::globalInstance();
      EXPECT_TRUE(rtThreadPool::mGlobalInstance == p);
      delete p;
      EXPECT_TRUE(rtThreadPool::mGlobalInstance == NULL);
      rtThreadPool::mGlobalInstance = globInst;
    }

    void raisePriorityTest()
    {
      rtThreadPool p(0);
      rtString s;
      p.raisePriority(s);
      EXPECT_TRUE(p.mRunning == true);
    }
};

TEST_F(rtThreadPoolTest, rtThreadPoolTests)
{
  creationTest();
  destructionNonGlobalTest();
  destructionGlobalTest();
  raisePriorityTest();
}
