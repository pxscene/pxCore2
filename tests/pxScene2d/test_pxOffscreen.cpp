#include <sstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"  // Brings in Google Mock.
#define private public
#define protected public
#include "pxCore.h"
#include "pxOffscreen.h"


int validateWidth; 
int validateHeight;
pxColor validateColor;

// Mock for pxOffscreen so that we can test that initWithColor calls 
// functions with correct parameters
 class MockPxOffscreen : public pxOffscreen {

 public:

    MockPxOffscreen() {}

    pxError init(int width, int height) 
    { 
      //printf("MOCK init\n");
      EXPECT_TRUE(width == validateWidth);
      EXPECT_TRUE(height == validateHeight);

      return PX_OK;
    }
    void fill(const pxColor& color)
    {
      //printf("MOCK fill\n");
      EXPECT_TRUE(color.u == validateColor.u);
    }

}; 

MockPxOffscreen  mockOffscreen;


TEST(pxOffscreenTest, pxOffscreenInitWithColorTest)
{
    validateWidth = 50;
    validateHeight = 60;
    validateColor = pxBlue;
    mockOffscreen.initWithColor(validateWidth,validateHeight,validateColor);

}

