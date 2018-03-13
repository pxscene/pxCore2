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
#include "rtString.h"
#include "rtLog.h"

#include "test_includes.h" // Needs to be included last

size_t buffSize = 1024;
char reallyLongString[2000]; 
void populateString() {
  int size = sizeof(reallyLongString);
  for(int i = 0; i < size; i++) {
    reallyLongString[i] = 'k';
  }
  reallyLongString[size-1] = '\0';
}

size_t testNum = 0;
void myRtLogHandler(rtLogLevel level, const char* /*file*/, int /*line*/, int /*threadId*/, char* message)
{
  switch(testNum) {
    case 1:
      EXPECT_TRUE(level == RT_LOG_DEBUG);
      break;
    case 2: 
      EXPECT_TRUE(level == RT_LOG_ERROR);
      break;
    case 3:
      // Should never happen - test 3 is INFO while setting is at WARN
      ASSERT_TRUE(false);
      break;
    case 4:
      // validate LEVEL and string
      EXPECT_TRUE(level == RT_LOG_INFO);
      EXPECT_TRUE(strnlen(message,1035) == buffSize-1); 
      break;
  }


}
class rtLogTest : public testing::Test
{
  public:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {
    }

};

TEST_F(rtLogTest, rtLogLevelFromStringTest)
{
  EXPECT_TRUE(rtLogLevelFromString("DEBUG") == RT_LOG_DEBUG);
  EXPECT_TRUE(rtLogLevelFromString("debug") == RT_LOG_DEBUG);
  EXPECT_TRUE(rtLogLevelFromString("INFO") == RT_LOG_INFO);
  EXPECT_TRUE(rtLogLevelFromString("info") == RT_LOG_INFO);
  EXPECT_TRUE(rtLogLevelFromString("WARN") == RT_LOG_WARN);
  EXPECT_TRUE(rtLogLevelFromString("warn") == RT_LOG_WARN);
  EXPECT_TRUE(rtLogLevelFromString("ERROR") == RT_LOG_ERROR);
  EXPECT_TRUE(rtLogLevelFromString("error") == RT_LOG_ERROR);
  EXPECT_TRUE(rtLogLevelFromString("FATAL") == RT_LOG_FATAL);
  EXPECT_TRUE(rtLogLevelFromString("fatal") == RT_LOG_FATAL);
}

TEST_F(rtLogTest, rtLogHandlerTest) {
  // Set to use the test/verification handler
  rtLogSetLogHandler(myRtLogHandler);
  testNum = 1;
  rtLogSetLevel(RT_LOG_DEBUG);
  rtLogDebug("Test a debug log message with a %s parm","something");

  testNum = 2;
  rtLogSetLevel(RT_LOG_INFO);
  rtLogError("Test a Error log message with a %s parm","some string");

  testNum = 3;
  rtLogSetLevel(RT_LOG_WARN);
  rtLogInfo("Test an info log message that should not be logged with a %s parm","some string");
  
  // Test extra large string truncation
  testNum = 4;
  populateString();
  rtLogSetLevel(RT_LOG_INFO);
  rtLogInfo("%s\n",reallyLongString);
  
  // Reset to default handler
  rtLogSetLogHandler(NULL);
}
