#include <sstream>

#include "pxScene2d.h"
#include "pxContext.h"
#include "pxWindow.h"
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

  void test_scene_screenshot()
  {
    pxScene2d* scene = new pxScene2d(true, NULL);
    rtString type("image/png;base64");
    rtString pngData;
    EXPECT_EQ ((int)RT_OK, (int)scene->screenshot(type, pngData));
    EXPECT_GT ((int)pngData.length(), 0);
    delete scene;
  }

  void test_base64_encode()
  {
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
    int fbo_w = 101;
    int fbo_h = 102;
    pxContext c;
    c.init();
    pxContextFramebufferRef f = c.createFramebuffer(fbo_w,fbo_h,false);
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
    GLint mode;
    glGetIntegerv(GL_READ_BUFFER, &mode);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    c.snapshot(o);
    glReadBuffer(mode);

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
  }
};

TEST_F(screenshotTest, screenshotTests)
{
  test_scene_screenshot();
  test_base64_encode();
  test_base64_encode_decode();
  test_pxStorePNGImage_empty();
  test_pxStorePNGImage_zero();
  test_pxStorePNGImage_normal();
  test_pixels();
}
