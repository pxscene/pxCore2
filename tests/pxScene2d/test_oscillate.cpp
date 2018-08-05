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

class oscillateTests : public testing::Test
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
      startJsFile("test_OSCILLATE.js");
    }

private:
    void startJsFile(const char *jsfile)
    {
      mUrl  = jsfile;
      mView = new pxScriptView(mUrl,"");
    }

    void process(float nosecs)
    {
      double  secs = pxSeconds();
      while ((pxSeconds() - secs) < nosecs)
      {
        if (NULL != mView)
        {
          mView->onUpdate(pxSeconds());
          script.pump();
        }
      }
    }

    void test()
    {
      process(5.0);
      rtObjectRef   scene = mView->mScene;
      mScene = (pxScene2d*)scene.getPtr();
      ASSERT_NE(mScene, nullptr);
      mRoot = mScene->getRoot();
      rtObjectRef obj0 = pxObject::getObjectById("box0", mRoot);
      EXPECT_TRUE(NULL != obj0.getPtr());
      rtObjectRef obj1 = pxObject::getObjectById("box1", mRoot);
      EXPECT_TRUE(200 == ((pxObject*)obj1.getPtr())->mx);
      rtObjectRef obj2 = pxObject::getObjectById("box2", mRoot);
      EXPECT_TRUE(0 != ((pxObject*)obj2.getPtr())->mx);
      rtObjectRef obj3 = pxObject::getObjectById("box3", mRoot);
      EXPECT_TRUE(200 == ((pxObject*)obj3.getPtr())->mx);
    }

private:
    pxObject*     mRoot;
    pxScriptView* mView;
    pxScene2d* mScene;
    rtString      mUrl;
}; // CLASS - oscillateTests


TEST_F(oscillateTests, oscillateTest)
{
    test();
}
