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

#include "test_includes.h" // Needs to be included last

using namespace std;

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
};

TEST_F(pxScene2dTest, pxScene2dTests)
{
    getArchiveTest();
    viewContainerTest();
    initFromUrlFromParentTest();
    initFromUrlFromLocalTest();
}
