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
#include "rtString.h"
#include "pxScene2d.h"
#include "pxImage.h"
#include "pxResource.h"
#include "rtPromise.h"
#include <string.h>
#include <sstream>

#include "test_includes.h" // Needs to be included last

#define IMAGE_URL "https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg"
#define IMAGE_WIDTH 246
#define IMAGE_HEIGHT 164

// Mock for imageResource so that we don't have to wait for real download
class MockImageResource : public rtImageResource {

 public:

    MockImageResource(const char* url = 0) { setUrl(IMAGE_URL); UNUSED_PARAM(url); }

    rtError w(int32_t& v) const override { v = IMAGE_WIDTH; return RT_OK;}
    rtError h(int32_t& v) const override { v = IMAGE_HEIGHT; return RT_OK; }
    int32_t w() const override { return IMAGE_WIDTH;}
    int32_t h() const override { return IMAGE_HEIGHT;}
    rtError description(rtString& d) const { d = "rtImageResource"; return RT_OK; }
};

class pxImageTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
        mScene = new pxScene2d(false);
        mImage = new pxImage(mScene);
    }

    virtual void TearDown()
    {
    }

    void pxImageOnScreenWidthTest()
    {
        MockImageResource * imageRes = new MockImageResource(IMAGE_URL);
        rtObjectRef root = mScene->get<rtObjectRef>("root");
       
        mImage.set("resource", imageRes);
        mImage.set("stretchX", pxConstantsStretch::NONE);
        mImage.set("stretchY", 0);
        mImage.set("parent", root);
        pxImage* pImage = (pxImage*)mImage.getPtr();

        EXPECT_TRUE(pImage->getOnscreenWidth() == IMAGE_WIDTH);
        mImage.set("stretchX", 1);
        EXPECT_TRUE(pImage->getOnscreenWidth() == IMAGE_WIDTH);
        mImage.set("stretchX", 2);
        EXPECT_TRUE(pImage->getOnscreenWidth() == IMAGE_WIDTH);
        // Now, set explicit w for image and test values returned
        // for each stretch setting
        mImage.set("w", IMAGE_WIDTH * 3);
        mImage.set("stretchX", 0);
        EXPECT_TRUE(pImage->getOnscreenWidth() == IMAGE_WIDTH);
        mImage.set("stretchX", 1);
        EXPECT_TRUE(pImage->getOnscreenWidth() == IMAGE_WIDTH * 3);
        mImage.set("stretchX", 2);
        EXPECT_TRUE(pImage->getOnscreenWidth() == IMAGE_WIDTH * 3);
    }
    void pxImageOnScreenHeightTest()
    {
        MockImageResource * imageRes = new MockImageResource(IMAGE_URL);
        rtObjectRef root = mScene.get<rtObjectRef>("root");
        mImage.set("resource", imageRes);
        mImage.set("stretchX", 0);
        mImage.set("stretchY", 0);
        mImage.set("parent", root);
        // Test that height returned for the image is resource h
        // since w and h are not set explicitly
        pxImage* pImage = (pxImage*)mImage.getPtr();
        EXPECT_TRUE(pImage->getOnscreenHeight() == IMAGE_HEIGHT);
        mImage.set("stretchY", 1);
        EXPECT_TRUE(pImage->getOnscreenHeight() == IMAGE_HEIGHT);
        mImage.set("stretchY", 2);
        EXPECT_TRUE(pImage->getOnscreenHeight() == IMAGE_HEIGHT);
        // Now, set explicit h for image and test values returned
        // for each stretch setting
        mImage.set("h", IMAGE_HEIGHT * 3);
        mImage.set("stretchY", 0);
        EXPECT_TRUE(pImage->getOnscreenHeight() == IMAGE_HEIGHT);
        mImage.set("stretchY", 1);
        EXPECT_TRUE(pImage->getOnscreenHeight() == IMAGE_HEIGHT * 3);
        mImage.set("stretchY", 2);
        EXPECT_TRUE(pImage->getOnscreenHeight() == IMAGE_HEIGHT * 3);
    }

    void pxImageCreateFailedTest()
    {
      pxScene2d* scene = mScene.getPtr();
      scene->mDisposed = true;
      rtObjectRef param;
      rtObjectRef img;
      EXPECT_TRUE(RT_FAIL == scene->create(param, img));
      scene->mDisposed = false;
    }

    void pxImageLoadFromArchiveTest()
    {
      pxScene2d* scene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == scene->loadArchive("supportfiles/test_arc_resources.jar", archive));
      pxImage* image = new pxImage(scene);
      image->setUrl("images/status_bg.svg");
      rtString key("supportfiles/test_arc_resources.jar_images/status_bg.svg");
      ImageMap::iterator it = pxImageManager::mImageMap.find(key.cString());
      EXPECT_TRUE (it != pxImageManager::mImageMap.end());
      rtString name = (((pxResource*)(image->mResource.getPtr()))->mName);
      EXPECT_TRUE (strcmp(name.cString(), "supportfiles/test_arc_resources.jar_images/status_bg.svg") == 0);
    }
    pxScene2dRef mScene;
    rtObjectRef mImage;
};

TEST_F(pxImageTest, pxImageCompleteTest)
{
    pxImageOnScreenWidthTest();
    pxImageOnScreenHeightTest();
    pxImageCreateFailedTest();
    pxImageLoadFromArchiveTest();
}

class rtImageResourceTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void rtImageResourceLoadFromArchiveSuccessTest()
    {
      rtImageResource res("images/status_bg.svg");
      pxScene2d* scene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == scene->loadArchive("supportfiles/test_arc_resources.jar", archive));
      res.loadResourceFromArchive(scene->getArchive());
      rtObjectRef loadStatus = new rtMapObject;
      res.loadStatus(loadStatus);
      rtValue status;
      loadStatus.Get("statusCode",&status);
      EXPECT_TRUE(status == PX_RESOURCE_STATUS_OK);
      rtString key("supportfiles/test_arc_resources.jar_images/status_bg.svg");
      ImageMap::iterator it = pxImageManager::mImageMap.find(key.cString());
      EXPECT_TRUE (it != pxImageManager::mImageMap.end());
      delete scene;
    }

    void rtImageResourceLoadFromArchiveFailureTest()
    {
      rtImageResource res("images/status_bg1.svg");
      pxScene2d* scene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == scene->loadArchive("supportfiles/test_arc_resources.jar", archive));
      res.loadResourceFromArchive(scene->getArchive());
      rtObjectRef loadStatus = new rtMapObject;
      res.loadStatus(loadStatus);
      rtValue status;
      loadStatus.Get("statusCode",&status);
      EXPECT_TRUE(status == PX_RESOURCE_STATUS_FILE_NOT_FOUND);
      delete scene;
    }
};

TEST_F(rtImageResourceTest, rtImageResourcesTest)
{
    rtImageResourceLoadFromArchiveSuccessTest();
    rtImageResourceLoadFromArchiveFailureTest();
}

class rtImageAResourceTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void rtImageAResourceLoadFromArchiveTest()
    {
      rtImageAResource res("images/status_bg.svg");
      pxScene2d* scene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == scene->loadArchive("supportfiles/test_arc_resources.jar", archive));
      res.loadResourceFromArchive(scene->getArchive());
      rtObjectRef loadStatus = new rtMapObject;
      res.loadStatus(loadStatus);
      rtValue status;
      loadStatus.Get("statusCode",&status);
      EXPECT_TRUE(status == PX_RESOURCE_STATUS_UNKNOWN_ERROR);
      delete scene;
    }
};

TEST_F(rtImageAResourceTest, rtImageAResourcesTest)
{
    rtImageAResourceLoadFromArchiveTest();
}

