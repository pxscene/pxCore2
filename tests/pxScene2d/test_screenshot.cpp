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
    printf("test_base64_encode \n");
    fflush(stdout);
    for (int i = 0; i<100; i++)
    {
      rtData pngData2;
      pngData2.init(i);
      size_t l;
      char* d = base64_encode(pngData2.data(), pngData2.length(), &l);
      EXPECT_EQ ((int)l, 4*((i+2)/3));
      EXPECT_TRUE (i == 0 || (NULL != d && *d != 0));
      if (NULL != d && *d != 0)
      {
        size_t sl = strlen(rtString(d, l).cString());
        EXPECT_EQ (sl, l);
        free(d);
      }
    }
  }

  void test_base64_encode_decode()
  {
    printf("test_base64_encode_decode \n");
    fflush(stdout);
    for (size_t i = 0; i<100; i++)
    {
      rtData pngData2;
      pngData2.init(i);
      size_t l;
      char* d = base64_encode(pngData2.data(), pngData2.length(), &l);
      EXPECT_EQ (l, 4*((i+2)/3));
      EXPECT_TRUE (l == 0 || NULL != d);

      if (NULL != d)
      {
        size_t l2;
        unsigned char *d2 = base64_decode((const unsigned char *)d, l, &l2);
        EXPECT_TRUE (l < 4 || NULL != d2);
        if (d2)
        {
          EXPECT_EQ (pngData2.length(), l2);
          int eq = memcmp(pngData2.data(), d2, pngData2.length());
          EXPECT_EQ (eq, 0);
          free(d2);
        }
        free(d);
      }
    }
  }

  void test_pxStorePNGImage_empty()
  {
    printf("test_pxStorePNGImage_empty \n");
    fflush(stdout);
    pxOffscreen o;
    o.setUpsideDown(true);
    rtData pngData2;
    EXPECT_EQ ((int)RT_OK, (int)pxStorePNGImage(o, pngData2));
    EXPECT_EQ ((int)pngData2.length(), 0);
  }

  void test_pxStorePNGImage_zero()
  {
    printf("test_pxStorePNGImage_zero \n");
    fflush(stdout);
    pxOffscreen o;
    EXPECT_EQ (RT_OK, o.init(0, 0));
    o.setUpsideDown(true);
    rtData pngData2;
    EXPECT_EQ ((int)RT_OK, (int)pxStorePNGImage(o, pngData2));
    EXPECT_EQ ((int)pngData2.length(), 0);
  }

  void test_pxStorePNGImage_normal()
  {
    printf("test_pxStorePNGImage_normal \n");
    fflush(stdout);
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
    printf("test_pixels \n");
    fflush(stdout);
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
    printf("test_pixels 1\n");
    fflush(stdout);
    if (RT_OK != e)
      return;

    e = c.setFramebuffer(f);
    EXPECT_EQ ((int)RT_OK, (int)e);
    printf("test_pixels 2\n");
    fflush(stdout);
    if (RT_OK != e)
      return;

    printf("test_pixels 3\n");
    fflush(stdout);
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

    printf("test_pixels 4\n");
    fflush(stdout);
    if (o.width() != fbo_w || o.height() != fbo_h)
      return;
    printf("test_pixels 5\n");
    fflush(stdout);

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

    printf("test_pixels 7\n");
    fflush(stdout);
    if (o2.width() != fbo_w || o2.height() != fbo_h)
      return;

    printf("test_pixels 8\n");
    fflush(stdout);
    pix1 = o2.pixel(0,0);
    EXPECT_EQ (lineColorRect[0]*255, pix1->r);
    EXPECT_EQ (lineColorRect[1]*255, pix1->g);
    EXPECT_EQ (lineColorRect[2]*255, pix1->b);

    pix2 = o2.pixel(fbo_w/2,fbo_h/2);
    EXPECT_EQ (fillColorRect[0]*255, pix2->r);
    EXPECT_EQ (fillColorRect[1]*255, pix2->g);
    EXPECT_EQ (fillColorRect[2]*255, pix2->b);

    delete win;
    printf("test_pixels 9\n");
    fflush(stdout);
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
