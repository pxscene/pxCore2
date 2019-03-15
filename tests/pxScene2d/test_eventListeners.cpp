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

    void runPendingListenerTest()
    {
      rtObjectRef e = new rtMapObject;
      mScene->mEmit.send("addPendingEvents",e);
      process();
      EXPECT_TRUE(mTestObj->mEmit->mPendingEntriesToAdd.size() == 0);
    }

    void sendSyncEventTest()
    {
      rtObjectRef e = new rtMapObject;
      size_t eventEntriesSizeBefore = mTestObj->mEmit->mEntries.size();
      mScene->mEmit.send("syncEvent",e);
      EXPECT_TRUE(eventEntriesSizeBefore+1 == mTestObj->mEmit->mEntries.size());
    }

    void sendAsyncEventTest()
    {
      rtObjectRef e = new rtMapObject;
      size_t eventEntriesSizeBefore = mTestObj->mEmit->mEntries.size();
      mScene->mEmit.sendAsync("asyncEvent",e);
      EXPECT_TRUE(eventEntriesSizeBefore == mTestObj->mEmit->mEntries.size());
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
    runPendingListenerTest();
    sendSyncEventTest();
    sendAsyncEventTest();
}
