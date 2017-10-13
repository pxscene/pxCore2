#include <sstream>

// #include "gtest/gtest.h"
#define private public
#define protected public
#include "pxCore.h"
#include "pxOffscreen.h"

#include "test_includes.h" // Needs to be included last

int validateWidth; 
int validateHeight;
pxColor validateColor;
pxOffscreen  offscreen;


TEST(pxOffscreenTest, pxOffscreenInitWithColorTest)
{
    validateWidth = 50;
    validateHeight = 60;
    validateColor = pxBlue;
    offscreen.initWithColor(validateWidth,validateHeight,validateColor);

    // test the color of offscreen buffer
    for (int32_t i = 0; i < validateHeight; i++)
    {
      pxPixel *p = offscreen.scanline(i);
      for (int32_t j = 0; j < validateWidth; j++)
      {
        EXPECT_TRUE((*p++).u == validateColor.u);
      }
    }
    
    // Test size of offscreen buffer
    pxRect rect = offscreen.size();

    EXPECT_EQ(rect.height(), validateHeight);
    EXPECT_EQ(rect.width(), validateWidth);


}

