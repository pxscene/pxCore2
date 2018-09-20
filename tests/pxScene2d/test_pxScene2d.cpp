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

#include <sstream>

#define private public
#define protected public

#include "pxScene2d.h"
#include "rtString.h"
#include <string.h>
#include <unistd.h>
#include "pxTimer.h"

#include "test_includes.h" // Needs to be included last

using namespace std;

extern void populateWaylandAppsConfig();
extern void populateAllAppsConfig();
extern void populateAllAppDetails(rtString& appDetails);
extern map<string, string> gWaylandAppsMap;
extern map<string, string> gWaylandRegistryAppsMap;
extern map<string, string> gPxsceneWaylandAppsMap;
extern rtScript script;

class pxScene2dTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
      
    }

    void getArchiveTest()
    {
      pxScene2d* scene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == scene->loadArchive("supportfiles/test_arc_resources.jar", archive));
      EXPECT_TRUE(archive == scene->getArchive());
      delete scene;
    }

    void viewContainerTest()
    {
      pxScene2d* parentscene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == parentscene->loadArchive("supportfiles/test_arc_resources.jar", archive));
      pxSceneContainer* container = new pxSceneContainer(parentscene);
      container->setUrl("supportfiles/helloworld.js");
      pxScriptView* view = (pxScriptView*) container->mScriptView.getPtr();
      EXPECT_TRUE (NULL != view);
      rtValue scene;
      rtValue args;
      args.setString("scene");
      EXPECT_TRUE(RT_OK == pxScriptView::getScene(1, &args, &scene, (void*)view));
      pxScene2d* opscene = (pxScene2d*)(scene.toObject().getPtr());
      EXPECT_TRUE (NULL != opscene);
      EXPECT_TRUE (opscene->viewContainer() == view->mViewContainer);
      delete parentscene;
    }

    void initFromUrlFromParentTest()
    {
      pxScene2d* parentscene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == parentscene->loadArchive("supportfiles/test_arc_resources.jar", archive));
      pxSceneContainer* container = new pxSceneContainer(parentscene);
      container->setUrl("mainfile.js");
      pxScriptView* view = (pxScriptView*) container->mScriptView.getPtr();
      EXPECT_TRUE (NULL != view);
      rtValue scene;
      rtValue args;
      args.setString("scene");
      EXPECT_TRUE(RT_OK == pxScriptView::getScene(1, &args, &scene, (void*)view));
      pxScene2d* opscene = (pxScene2d*)(scene.toObject().getPtr());
      EXPECT_TRUE (NULL != opscene);
      EXPECT_TRUE (opscene->getArchive() == parentscene->getArchive());
    }

    void initFromUrlFromLocalTest()
    {
      pxScene2d* parentscene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == parentscene->loadArchive("supportfiles/helloworld.js", archive));
      pxSceneContainer* container = new pxSceneContainer(parentscene);
      container->setUrl("helloworld.js");
      pxScriptView* view = (pxScriptView*) container->mScriptView.getPtr();
      EXPECT_TRUE (NULL != view);
      rtValue scene;
      rtValue args;
      args.setString("scene");
      EXPECT_TRUE(RT_OK == pxScriptView::getScene(1, &args, &scene, (void*)view));
      pxScene2d* opscene = (pxScene2d*)(scene.toObject().getPtr());
      EXPECT_TRUE (NULL != opscene);
      EXPECT_TRUE (opscene->getArchive() != parentscene->getArchive());
    }
    void populateWaylandAppsConfigTest()
    { 
      populateWaylandAppsConfig();
      EXPECT_TRUE ( 0 == gWaylandRegistryAppsMap.size());
      setenv("WAYLAND_APPS_CONFIG", "../../examples/pxScene2d/src/waylandregistry.conf", 1);
      populateWaylandAppsConfig();
      EXPECT_TRUE ( 4 == gWaylandRegistryAppsMap.size());
      setenv("WAYLAND_APPS_CONFIG", "../../tests/pxScene2d/supportfiles/jsonParseError.json", 1);
      populateWaylandAppsConfig();
    }
    void populateAllAppsConfigTest()
    {
      populateAllAppsConfig();
      setenv("PXSCENE_APPS_CONFIG", "../../tests/pxScene2d/supportfiles/pxsceneappregistry.conf", 1);
      populateAllAppsConfig();
      setenv("PXSCENE_APPS_CONFIG", "../../tests/pxScene2d/supportfiles/jsonParseError.json", 1);
      populateAllAppsConfig();
    }
    void populateAllAppDetailsTest()
    {
      rtString appDetails;
      populateAllAppDetails(appDetails);
      
      setenv("PXSCENE_APPS_CONFIG", "../../tests/pxScene2d/supportfiles/fileNotPresent.json", 1);
      populateAllAppDetails(appDetails);
      
      setenv("PXSCENE_APPS_CONFIG", "../../tests/pxScene2d/supportfiles/jsonParseError.json", 1);
      populateAllAppDetails(appDetails);
      
      setenv("PXSCENE_APPS_CONFIG", "../../tests/pxScene2d/supportfiles/pxsceneappregistry.conf", 1);
      populateAllAppsConfig();
      
      setenv("WAYLAND_APPS_CONFIG", "../../examples/pxScene2d/src/waylandregistry.conf", 1);
      populateWaylandAppsConfig();
      populateAllAppDetails(appDetails);
 
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
 
    void pxObjectTest()
    {
      mUrl = "test_OSCILLATE.js";
      mView = new pxScriptView(mUrl,"");
      process(5.0);
      rtObjectRef   scene = mView->mScene;
      pxScene2d* sceneptr = (pxScene2d*)scene.getPtr();
      ASSERT_NE(sceneptr, nullptr); 
      
      mRoot = sceneptr->getRoot();
      vector<rtRef<pxObject> > childrenVector = mRoot->mChildren ;
      EXPECT_TRUE ( RT_OK == childrenVector[2]->moveToBack()); 
      EXPECT_TRUE ( RT_OK == childrenVector[2]->moveToFront()); 
      
      EXPECT_TRUE ( RT_OK == childrenVector[3]->moveForward());
      EXPECT_TRUE ( RT_OK == childrenVector[3]->moveBackward());
      childrenVector[2]->createSnapshotOfChildren();
      childrenVector[3]->releaseData(false);
      childrenVector[6]->reloadData(true);
      EXPECT_TRUE ( RT_OK == childrenVector[2]->remove());
      EXPECT_TRUE ( RT_OK == childrenVector[3]->removeAll());
      mRoot->reloadData(false);
      mRoot->reloadData(true);
      EXPECT_TRUE ( RT_OK == mRoot->setPainting(true));  
      EXPECT_TRUE ( RT_OK == mRoot->setPainting(false));

      rtObjectRef props = new rtMapObject();
      props.set("t","image");
      rtObjectRef animateObjTest;
      EXPECT_TRUE ( RT_OK == mRoot->animateToObj(props, 20, 0, 1, 1, animateObjTest));


      EXPECT_TRUE(false == sceneptr->onMouseDown(3, 2, 0));
      EXPECT_TRUE(false == sceneptr->onMouseUp(3, 2, 0));
      EXPECT_TRUE(false == sceneptr->onKeyDown(3, 0));
      EXPECT_TRUE(false == sceneptr->onKeyUp(3, 0));
      EXPECT_TRUE(false == sceneptr->onChar(65));
      
    }
   
    void pxScene2dClassTest()
    {
      mUrl = "test_OSCILLATE.js";
      mView = new pxScriptView(mUrl,"");
      process(5.0); 
      rtObjectRef   scene = mView->mScene;
      pxScene2d* scenePtr = (pxScene2d*)scene.getPtr();
      ASSERT_NE(scenePtr, nullptr);
      
      mRoot = scenePtr->getRoot();
      rtValue  v; 
      bool flag; 
      EXPECT_TRUE ( RT_OK == scenePtr->collectGarbage());
      setenv("SPARK_ENABLE_COLLECT_GARBAGE", "1", 1);
      EXPECT_TRUE ( RT_OK == scenePtr->collectGarbage());
 
      EXPECT_TRUE ( RT_OK == scenePtr->suspend(v, flag));
      EXPECT_TRUE ( RT_OK == scenePtr->resume(v, flag));

      flag = true;
      bool setFlag = false;
      EXPECT_TRUE ( RT_OK == scenePtr->showOutlines(flag));
      EXPECT_TRUE ( RT_OK == scenePtr->setShowOutlines(setFlag));
      EXPECT_TRUE ( RT_OK == scenePtr->showDirtyRect(flag));
      EXPECT_TRUE ( RT_OK == scenePtr->setShowDirtyRect(setFlag));

      rtFunctionRef cAnimator; 
      EXPECT_TRUE ( RT_OK == scenePtr->customAnimator(cAnimator));

      rtString type = "image/png;base64";
      rtString pngData;
      EXPECT_TRUE ( RT_OK == scenePtr->screenshot(type, pngData));

      rtObjectRef imageA = new rtMapObject();
      rtObjectRef cImageA = new rtMapObject();
      imageA.set("t","imageA");
      EXPECT_TRUE ( RT_OK == scenePtr->createImageA(imageA, cImageA));
      EXPECT_TRUE ( RT_OK == scenePtr->createImage9Border(imageA, cImageA));
      EXPECT_TRUE ( RT_OK == scenePtr->createImageAResource(imageA, cImageA));
    } 
  private:
    pxObject*     mRoot;
    pxScriptView* mView;
    rtString      mUrl;
}; 

TEST_F(pxScene2dTest, pxScene2dTests)
{
    getArchiveTest();
    viewContainerTest();
    initFromUrlFromParentTest();
    initFromUrlFromLocalTest();
    populateWaylandAppsConfigTest();
    populateAllAppsConfigTest();
    populateAllAppDetailsTest();
    pxObjectTest();
    pxScene2dClassTest();
}
