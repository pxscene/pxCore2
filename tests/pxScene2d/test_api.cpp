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



class pxApiTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void runApiTests()
    {
      startJsFile("onescene_with_parent.js");

      runGetChildTests();
    }

private:
    void startJsFile(const char *jsfile)
    {
      mUrl  = jsfile;
      mView = new pxScriptView(mUrl,"");
    }

    void runGetChildTests()
    {
      rtObjectRef   scene = mView->mScene;
      pxScene2d* sceneptr = (pxScene2d*)scene.getPtr();

      ASSERT_NE(sceneptr, nullptr);

      mRoot = sceneptr->getRoot();

      rtObjectRef obj;

      // Beyond START >> RT_ERROR_INVALID_ARG
      //
      rtError err1 = mRoot->getChild(-1, obj);
      EXPECT_EQ(err1, RT_ERROR_INVALID_ARG);

      // Beyond END >> RT_ERROR_INVALID_ARG
      //
      rtError err2 = mRoot->getChild(100, obj);
      EXPECT_EQ(err2, RT_ERROR_INVALID_ARG);
    }

private:
    pxObject*     mRoot;
    pxScriptView* mView;
    rtString      mUrl;

}; // CLASS - pxApiTest


TEST_F(pxApiTest, apiTest)
{
   runApiTests();
}
