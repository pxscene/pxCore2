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

#include "pxTimer.h"
#include "test_includes.h" // Needs to be included last

using namespace std;

#define TEST_TIME 2

class pxTimerNativeTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void testpxTimerNative()
    {
        double startTime = 0.0;
        double endTime = 0.0;
        double opTime = 0.0;
        
        opTime = pxSeconds();
        EXPECT_TRUE(opTime > 0);
 
        opTime = pxMilliseconds();
        EXPECT_TRUE(opTime > 0);
        
        opTime = pxMicroseconds();
        EXPECT_TRUE(opTime > 0);
 
        startTime = pxSeconds();
        pxSleepMS(TEST_TIME * 1000);
        endTime = pxSeconds();
        EXPECT_TRUE((int)(endTime - startTime) == TEST_TIME);
    }

};

TEST_F(pxTimerNativeTest, pxTimerNativeTestS)
{
    testpxTimerNative();
}
