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

