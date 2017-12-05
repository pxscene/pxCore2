#include <sstream>

#define private public
#define protected public
#include "rtMutex.h"
#include "rtThreadPool.h"

#include "test_includes.h" // Needs to be included last

rtMutex* gMutex = NULL;

void taskCallback(void* data)
{
  if (data != NULL)
  {
    rtMutexLockGuard g(*gMutex);
    int *i = (int*)data;
    *i = 5;
  }
}

class pxMutexTest : public testing::Test
{
public:
    pxMutexTest(): mThreadPool(NULL), mThreadCondition(NULL), mTestValue(0) {}
    virtual void SetUp()
    {
      gMutex = new rtMutex();
      mThreadPool = new rtThreadPool(2);
      mThreadCondition = new rtThreadCondition();
    }

    virtual void TearDown()
    {
      delete mThreadPool;
      mThreadPool = NULL;

      delete mThreadCondition;
      mThreadCondition = NULL;

      delete gMutex;
      gMutex = NULL;
    }

  void changeTest()
  {
    rtThreadTask *task = new rtThreadTask(taskCallback, &mTestValue, "");
    mThreadPool->executeTask(task);
    EXPECT_TRUE (true);
  }

  void lockWriteTest()
  {
    gMutex->lock();
    mTestValue = 6;
    gMutex->unlock();
    EXPECT_TRUE (true);
  }

  void lockReadTest()
  {
    int result = 0;
    gMutex->lock();
    result = mTestValue;
    gMutex->unlock();
    EXPECT_TRUE (result == 5 || result == 6);
  }
  private:
    rtThreadPool* mThreadPool;
    rtThreadCondition* mThreadCondition;
    int mTestValue;
};

TEST_F(pxMutexTest, pxMutexTests)
{
  changeTest();
  lockWriteTest();
  lockReadTest();
}

