#ifndef ENABLE_RT_NODE
#define ENABLE_RT_NODE
#endif

#include <sstream>

#define private public
#define protected public

#include "pxScene2d.h"
#include <string.h>
#include "pxIView.h"
#include "pxTimer.h"
#include "rtObject.h"
#include "rtString.h"

#include <rtRef.h>

#include "test_includes.h" // Needs to be included last

using namespace std;

extern rtScript script;



class eventListenerTests : public testing::Test
{
  public:
    virtual void SetUp()
    {
	initialize();
    }

    virtual void TearDown()
    {
    }

    void initialize()
    {
      startJsFile("eventListeners.js");
      process();

      rtObjectRef   scene = mView->mScene;
      mScene = (pxScene2d*)scene.getPtr();
      ASSERT_NE(mScene, nullptr);
      mRoot = mScene->getRoot();
      rtObjectRef obj;
      rtError err1 = mRoot->getChild(0, obj);
      EXPECT_EQ(err1, RT_OK);
      mTestObj = (pxObject *)obj.getPtr();
    }

private:
    void startJsFile(const char *jsfile)
    {
      mUrl  = jsfile;
      mView = new pxScriptView(mUrl,"");
    }

    void process()
    {
      double  secs = pxSeconds();
      while ((pxSeconds() - secs) < 1.0)
      {
        if (NULL != mView)
        {
          mView->onUpdate(pxSeconds());
          script.pump();
        }
      }
    }

    void runAddListenerTest()
    {
      rtObjectRef e = new rtMapObject;
      mScene->mEmit.send("addEventsProper",e);
      process();
      EXPECT_TRUE(mTestObj->mEmit->mEntries.size() == 1);
    }

    void runDelListenerImproperTest()
    {
      rtObjectRef e = new rtMapObject;
      mScene->mEmit.send("removeEventsImProper",e);
      process();
      EXPECT_TRUE(mTestObj->mEmit->mEntries.size() == 1);
    }

    void runDelListenerProperTest()
    {
      rtObjectRef e = new rtMapObject;
      mScene->mEmit.send("removeEventsProper",e);
      process();
      EXPECT_TRUE(mTestObj->mEmit->mEntries.size() == 0);
    }
private:
    pxObject*     mRoot;
    pxScriptView* mView;
    pxScene2d* mScene;
    rtString      mUrl;
    pxObject* mTestObj;
}; // CLASS - eventListenerTests


TEST_F(eventListenerTests, eventListenerTest)
{
    runAddListenerTest();
    runDelListenerImproperTest();
    runDelListenerProperTest();
}
