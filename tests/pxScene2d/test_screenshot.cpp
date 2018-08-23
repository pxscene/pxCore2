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

#include "pxScene2d.h"
#include "pxContext.h"
#include "pxWindow.h"
#include "pxUtil.h"
#include "test_includes.h" // Needs to be included last

class screenshotTest : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void test_base64_encode()
  {
    for (int i = 0; i<100; i++)
    {
      rtData pngData2;
      pngData2.init(i);
      rtString out;

      rtError res = base64_encode(pngData2, out);

printf("\n >>>>>>>>>>>>>>>>>>>>> [%d]  len = %d   res = %d",i, out.length(), res);

      EXPECT_TRUE (res == (i == 0) ? RT_FAIL : RT_OK);
      EXPECT_EQ ((int)out.length(), 4*((i+2)/3));
      EXPECT_TRUE (i == 0 || (NULL != out.cString() && out.length() != 0));
    }
  }

  void test_base64_encode_decode()
  {
    for (size_t i = 0; i<100; i++)
    {
      rtData pngData2;
      pngData2.init(i);

      rtString s1;

      rtError res1 = base64_encode(pngData2.data(), pngData2.length(), s1);

      EXPECT_TRUE (res1 == (i == 0) ? RT_FAIL : RT_OK);
      EXPECT_EQ ((size_t)s1.length(), 4*((i+2)/3));
      EXPECT_TRUE (s1.length() == 0 || NULL != s1.cString());

      if (NULL != s1.cString() )
      {
        rtData d2;
        rtError res2 = base64_decode(s1, d2);

        EXPECT_TRUE (res2 == (i == 0) ? RT_FAIL : RT_OK);
        EXPECT_TRUE (d2.length() < 4 || NULL != d2.data());
        if (d2.length() > 0)
        {
          EXPECT_EQ (pngData2.length(), d2.length());
          int eq = memcmp(pngData2.data(), d2.data(), pngData2.length());
          EXPECT_EQ (eq, 0);
        }
      }
    }
  }

  void test_pxStorePNGImage_empty()
  {
    pxOffscreen o;
    o.setUpsideDown(true);
    rtData pngData2;
    EXPECT_EQ ((int)RT_OK, (int)pxStorePNGImage(o, pngData2));
    EXPECT_EQ ((int)pngData2.length(), 0);
  }

  void test_pxStorePNGImage_zero()
  {
    pxOffscreen o;
    EXPECT_EQ (RT_OK, o.init(0, 0));
    o.setUpsideDown(true);
    rtData pngData2;
    EXPECT_EQ ((int)RT_OK, (int)pxStorePNGImage(o, pngData2));
    EXPECT_EQ ((int)pngData2.length(), 0);
  }

  void test_pxStorePNGImage_normal()
  {
    for (int i = 1; i<=100; i++)
    {
      pxOffscreen o;
      EXPECT_EQ ((int)RT_OK, (int)o.init(i, i));
      o.setUpsideDown(true);
      rtData pngData2;
      EXPECT_EQ ((int)RT_OK, (int)pxStorePNGImage(o, pngData2));
      EXPECT_GT ((int)pngData2.length(), 0);
    }
  }

  void test_pixels()
  {
    int fbo_w = 640;
    int fbo_h = 480;

    // init opengl
    pxWindow* win = new pxWindow();
    win->init(0,0,fbo_w,fbo_h);

    pxContext c;
    c.init();
    pxContextFramebufferRef f = c.createFramebuffer(fbo_w,fbo_h,true);
    rtError e = c.setFramebuffer(NULL);
    EXPECT_EQ ((int)RT_OK, (int)e);
    if (RT_OK != e)
      return;

    e = c.setFramebuffer(f);
    EXPECT_EQ ((int)RT_OK, (int)e);
    if (RT_OK != e)
      return;

    float fillColor[] = {0,0,0,0};
    c.clear(0,0,fillColor);
    float fillColorRect[4] = {1,0,1,1};
    float lineColorRect[4] = {1,1,0,1};
    c.drawRect(fbo_w,fbo_h,10,fillColorRect,lineColorRect);
    pxOffscreen o;
    c.snapshot(o);

    // verify image
    EXPECT_TRUE (o.upsideDown());
    EXPECT_EQ (fbo_w,o.width());
    EXPECT_EQ (fbo_h,o.height());

    if (o.width() != fbo_w || o.height() != fbo_h)
      return;

    pxPixel* pix1 = o.pixel(0,0);
    EXPECT_EQ (lineColorRect[0]*255, pix1->r);
    EXPECT_EQ (lineColorRect[1]*255, pix1->g);
    EXPECT_EQ (lineColorRect[2]*255, pix1->b);

    pxPixel* pix2 = o.pixel(fbo_w/2,fbo_h/2);
    EXPECT_EQ (fillColorRect[0]*255, pix2->r);
    EXPECT_EQ (fillColorRect[1]*255, pix2->g);
    EXPECT_EQ (fillColorRect[2]*255, pix2->b);

    // pxOffscreen <-> PNG
    rtData pngData2;
    EXPECT_EQ ((int)RT_OK, (int)pxStorePNGImage(o, pngData2));
    EXPECT_GT ((int)pngData2.length(), 0);
    pxOffscreen o2;
    EXPECT_EQ ((int)RT_OK, (int)pxLoadPNGImage((const char *)pngData2.data(), pngData2.length(), o2));

    // verify image
    EXPECT_FALSE (o2.upsideDown());
    EXPECT_EQ (fbo_w,o2.width());
    EXPECT_EQ (fbo_h,o2.height());

    if (o2.width() != fbo_w || o2.height() != fbo_h)
      return;

    pix1 = o2.pixel(0,0);
    EXPECT_EQ (lineColorRect[0]*255, pix1->r);
    EXPECT_EQ (lineColorRect[1]*255, pix1->g);
    EXPECT_EQ (lineColorRect[2]*255, pix1->b);

    pix2 = o2.pixel(fbo_w/2,fbo_h/2);
    EXPECT_EQ (fillColorRect[0]*255, pix2->r);
    EXPECT_EQ (fillColorRect[1]*255, pix2->g);
    EXPECT_EQ (fillColorRect[2]*255, pix2->b);

    delete win;
  }
};

TEST_F(screenshotTest, screenshotTests)
{
  test_base64_encode();
  test_base64_encode_decode();
  test_pxStorePNGImage_empty();
  test_pxStorePNGImage_zero();
  test_pxStorePNGImage_normal();
  test_pixels();
}
