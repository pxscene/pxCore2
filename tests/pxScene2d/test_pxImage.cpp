#include <sstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"  // Brings in Google Mock.
#define private public
#define protected public
#include "rtString.h"
#include "pxScene2d.h"
#include "pxImage.h"
#include "pxResource.h"
#include "rtPromise.h"
#include <string.h>
#include <sstream>

#define IMAGE_URL "https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg"
#define IMAGE_WIDTH 246
#define IMAGE_HEIGHT 164

// Mock for imageResource so that we don't have to wait for real download
class MockImageResource : public rtImageResource {

 public:

    MockImageResource(const char* url = 0) { setUrl(IMAGE_URL); }

    rtError w(int32_t& v) const { v = IMAGE_WIDTH; return RT_OK;}
    rtError h(int32_t& v) const { v = IMAGE_HEIGHT; return RT_OK; }
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

    pxScene2dRef mScene;
    rtObjectRef mImage;
};

TEST_F(pxImageTest, pxImageCompleteTest)
{
    pxImageOnScreenWidthTest();
    pxImageOnScreenHeightTest();
}
