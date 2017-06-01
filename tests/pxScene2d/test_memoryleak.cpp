#define ENABLE_RT_NODE


#define private public
#define protected public

#include "pxScene2d.h"
#include <string.h>
#include "pxIView.h"
#include "pxTimer.h"
#include "rtObject.h"
#include <rtRef.h>

#include "test_includes.h" // Needs to be included last

using namespace std;

extern rtNode script;

class pxSceneContainerLeakTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void withParentRemovedGCNotHappenedTest()
    {
      startJsFile("onescene_with_parent.js");
      process();
      populateObjects();

      pxObject* sceneContainer = mSceneContainer[0];
      sceneContainer->remove();
      EXPECT_TRUE (sceneContainer->mRefCount > 1);
      EXPECT_TRUE (sceneContainer->parent() == NULL);
      script.garbageCollect();
    }

    void withParentRemovedGCHappenedTest()
    {
      startJsFile("onescene_with_parent.js");
      process();
      populateObjects();

      pxObject* sceneContainer = mSceneContainer[0];
      sceneContainer->AddRef();
      sceneContainer->remove();
      script.garbageCollect();
      pxSleepMS(3000);
      EXPECT_TRUE (sceneContainer->mRefCount == 1);
      script.garbageCollect();
    }

    void withoutParentRemovedGCNotHappenedTest()
    {
      startJsFile("onescene_with_parent.js");
      process();
      populateObjects();

      pxObject* sceneContainer = mSceneContainer[0];
      EXPECT_TRUE (sceneContainer->mRefCount > 1);
      EXPECT_TRUE (sceneContainer->parent() != NULL);
      script.garbageCollect();
    }

    void withoutParentRemovedGCHappenedTest()
    {
      startJsFile("onescene_with_parent.js");
      process();
      populateObjects();

      pxObject* sceneContainer = mSceneContainer[0];
      script.garbageCollect();
      EXPECT_TRUE (sceneContainer->mRefCount > 1);
      EXPECT_TRUE (sceneContainer->parent() != NULL);
    }

private:

    void startJsFile(char *jsfile)
    {
      mUrl = jsfile;
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

    void populateObjects()
    {
      rtObjectRef scene = mView->mScene;
      pxScene2d* sceneptr = (pxScene2d*)scene.getPtr();
      mRoot = sceneptr->getRoot();
      int objcount = 0;
      for(vector<rtRefT<pxObject> >::iterator it = mRoot->mChildren.begin(); it != mRoot->mChildren.end(); ++it)
      {
        if (strcmp((*it)->getMap()->className,"pxSceneContainer") == 0)
        {
          mSceneContainer[objcount] = (*it).getPtr();
        }
      }
    }

    pxObject* mRoot;
    pxObject* mSceneContainer[];
    pxScriptView* mView;
    rtString mUrl;
};

/*TEST_F(pxSceneContainerLeakTest, sceneContainerTest)
{
  withParentRemovedGCNotHappenedTest();
  withParentRemovedGCHappenedTest();
  withoutParentRemovedGCNotHappenedTest();
  withoutParentRemovedGCHappenedTest();
}*/
