#include <sstream>

#define private public
#define protected public
#include "rtString.h"
#include "pxScene2d.h"
#include "pxImage9Border.h"
#include <string.h>
#include <sstream>

#include "test_includes.h" // Needs to be included last

#define BORDER_LEFT_VALUE 50
#define BORDER_TOP_VALUE 51
#define BORDER_RIGHT_VALUE 52
#define BORDER_BOTTOM_VALUE 53

class TestImageBorder : public pxImage9Border
{
public:
  TestImageBorder(pxScene2d* scene) : pxImage9Border(scene){}
  virtual ~TestImageBorder() {}
  void testDraw() { draw(); }
};

class pxImageBorderTest : public testing::Test
{
public:
  virtual void SetUp()
  {
    mScene = new pxScene2d(false);
    mImageBorder = new pxImage9Border(mScene);
  }

  virtual void TearDown()
  {
  }

  void pxImage9BorderProperties()
  {
    float maskColor[4] = {1.0, 0.0, 0.0, 1.0};
    mImageBorder.set("borderLeft", BORDER_LEFT_VALUE);
    mImageBorder.set("borderTop", BORDER_TOP_VALUE);
    mImageBorder.set("borderRight", BORDER_RIGHT_VALUE);
    mImageBorder.set("borderBottom", BORDER_BOTTOM_VALUE);
    mImageBorder.set("maskColor", maskColor);
    mImageBorder.set("drawCenter", true);
    pxImage9Border* pImageBorder = (pxImage9Border*)mImageBorder.getPtr();
    float value = 0;
    rtError result = pImageBorder->borderLeft(value);
    EXPECT_TRUE(result == RT_OK && value ==  BORDER_LEFT_VALUE);
    result = pImageBorder->borderTop(value);
    EXPECT_TRUE(result == RT_OK && value ==  BORDER_TOP_VALUE);
    result = pImageBorder->borderRight(value);
    EXPECT_TRUE(result == RT_OK && value ==  BORDER_RIGHT_VALUE);
    result = pImageBorder->borderBottom(value);
    EXPECT_TRUE(result == RT_OK && value ==  BORDER_BOTTOM_VALUE);
    uint32_t colorValue = 0;
    result = pImageBorder->maskColor(colorValue);
    EXPECT_TRUE(result == RT_OK);
    bool drawCenterValue1 = false;
    bool drawCenterValue2 = false;
    result = pImageBorder->drawCenter(drawCenterValue1);
    drawCenterValue2 = pImageBorder->drawCenter();
    EXPECT_TRUE(result == RT_OK && drawCenterValue1 == true && drawCenterValue2 == true);
  }

  void pxImage9BorderTestDraw()
  {
    rtObjectRef testImageBorder = new TestImageBorder(mScene);
    TestImageBorder* pTestImageBorder = (TestImageBorder*)testImageBorder.getPtr();
    pTestImageBorder->testDraw();
  }

    pxScene2dRef mScene;
    rtObjectRef mImageBorder;
};

TEST_F(pxImageBorderTest, pxImageBorderCompleteTest)
{
  pxImage9BorderProperties();
  pxImage9BorderTestDraw();
}
