#include <sstream>

#define private public
#define protected public
#include "rtString.h"
#include "rtLog.h"

#include "test_includes.h" // Needs to be included last

size_t buffSize = 1024;
char reallyLongString[2000]; 
void populateString() {
  for(int i = 0; i < 2000; i++) {
    reallyLongString[i] = 'k';
  }
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
