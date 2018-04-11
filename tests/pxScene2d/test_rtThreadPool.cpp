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

    void destructionTest()
    {
      rtThreadPool *p = new rtThreadPool(0);
      delete p;
      EXPECT_TRUE(rtThreadPool::mGlobalInstance != NULL);
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
  destructionTest();
  raisePriorityTest();
}
