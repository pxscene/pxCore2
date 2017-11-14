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
