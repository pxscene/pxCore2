#include "pxTimer.h"

#include "test_includes.h" // Needs to be included last

using namespace std;

#define TEST_TIME 8

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
        pxSeconds();
        pxMilliseconds();
        pxMicroseconds();
        pxSleepMS(TEST_TIME);
    }

};

TEST_F(pxTimerNativeTest, pxTimerNativeTestS)
{
    testpxTimerNative();
}
