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

#include "pxCore.h"
#include "pxCanvas2d.h"
#include "pxColor.h"

#include "test_includes.h" // Needs to be included last

class pxCanvas2dExample :public pxCanvas2d
{
public:
    pxCanvas2dExample() : pxCanvas2d(), mWidth(0), mHeight(0) {}

private:
    int mWidth;
    int mHeight;

};

class pxCanvas2dTest : public testing::Test
{
public:
    virtual void SetUp()
    {
      canvas2d = new pxCanvas2dExample();

      canvas2d->init(256, 256);
    }

    virtual void TearDown()
    {
      canvas2d = NULL;
    }

  void matrixTest()
  {
    pxMatrix4T<float> m1; // identity
    pxMatrix4T<float> m2; // identity

    m1.translate(2, 2);

    canvas2d->setMatrix(m1);  // SET
    canvas2d->matrix(m2);     // GET

    float tx1 = m1.translateX();
    float tx2 = m2.translateX();

    EXPECT_TRUE (tx1 == tx2); // COMPARE

    m2.identity(); // reset

    canvas2d->setTextureMatrix(m1); // SET
    canvas2d->textureMatrix(m2);    // GET

    float tx3 = m1.translateX();
    float tx4 = m2.translateX();

    EXPECT_TRUE (tx3 == tx4); // COMPARE
  }

  void textureTest()
  {
    pxOffscreen tex;
    tex.init(256, 256);

    pxBuffer *pTex;

    canvas2d->setTexture(&tex);
    pTex = canvas2d->texture();

    EXPECT_TRUE (pTex == &tex);
  }

  void textureClampTest()
  {
    canvas2d->setTextureClamp(true);
    bool b1 = canvas2d->textureClamp();

    EXPECT_TRUE (b1 == true);

    canvas2d->setTextureClamp(false);
    bool b2 = canvas2d->textureClamp();

    EXPECT_TRUE (b2 == false);
  }

  void textureClampColorTest()
  {
    canvas2d->setTextureClampColor(true);
    bool b1 = canvas2d->textureClampColor();

    EXPECT_TRUE (b1 == true);

    canvas2d->setTextureClampColor(false);
    bool b2 = canvas2d->textureClampColor();

    EXPECT_TRUE (b2 == false);
  }

  void biLerpTest()
  {
    canvas2d->setBiLerp(true);
    bool b1 = canvas2d->biLerp();

    EXPECT_TRUE (b1 == true);

    canvas2d->setBiLerp(false);
    bool b2 = canvas2d->biLerp();

    EXPECT_TRUE (b2 == false);
  }

  void alphaTextureTest()
  {
    canvas2d->setAlphaTexture(true);
    bool b1 = canvas2d->alphaTexture();

    EXPECT_TRUE (b1 == true);

    canvas2d->setAlphaTexture(false);
    bool b2 = canvas2d->alphaTexture();

    EXPECT_TRUE (b2 == false);
  }

  void overdrawTest()
  {
    canvas2d->setOverdraw(true);
    bool b1 = canvas2d->overdraw();

    EXPECT_TRUE (b1 == true);

    canvas2d->setOverdraw(false);
    bool b2 = canvas2d->overdraw();

    EXPECT_TRUE (b2 == false);
  }

  void roundRectTest()
  {
    canvas2d->roundRect(0,0, 100,100, 10, 10);

    // How to test... read back pixels ?
  }

  void bufferInitTest()
  {
    pxError result1 = canvas2d->initWithBuffer(NULL);
    EXPECT_TRUE (result1 == PX_FAIL);

    pxOffscreen buffer;
    buffer.init(256, 256);

    pxError result2 = canvas2d->initWithBuffer(&buffer);
    EXPECT_TRUE (result2 == PX_OK);
  }

  void getPenTest()
  {
    double x = canvas2d->getPenX();
    double y = canvas2d->getPenY();

    EXPECT_TRUE (x == 0.0);
    EXPECT_TRUE (y == 0.0);
  }

  void testStrokeType()
  {
      canvas2d->setFillColor( pxColor(0xFF0000FF) );// RED
      canvas2d->setStrokeColor( pxColor(0xFF0000FF) );// RED
      canvas2d->setStrokeWidth(4.0);

      canvas2d->rectangle(50,50, 20,20);
      canvas2d->fill(); // FILL

      // Stoke Types
      pxCanvas2d::StrokeType st;

      canvas2d->setStrokeType(pxCanvas2d::StrokeType::inside);
      st = canvas2d->strokeType();
      canvas2d->stroke(); //STROKE

      EXPECT_TRUE (st == pxCanvas2d::StrokeType::inside);

      canvas2d->setStrokeType(pxCanvas2d::StrokeType::center);
      st = canvas2d->strokeType();
      canvas2d->stroke(); //STROKE

      EXPECT_TRUE (st == pxCanvas2d::StrokeType::center);

      canvas2d->setStrokeType(pxCanvas2d::StrokeType::outside);
      st = canvas2d->strokeType();
      canvas2d->stroke(); //STROKE

      EXPECT_TRUE (st == pxCanvas2d::StrokeType::outside);
  }

  void testColor()
  {
      canvas2d->setFillColor( pxColor(0xFF0000FF) );// RED
      canvas2d->setStrokeColor( pxColor(0xFF0000FF) );// RED
      canvas2d->setStrokeWidth(4.0);

      // Color
      pxColor c;

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      canvas2d->setFillColor( pxColor(0xFF0000FF) );// RED
      c = canvas2d->fillColor();
      canvas2d->fill(); //FILL

      EXPECT_TRUE (c.u == 0xFF0000FF);
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      canvas2d->setStrokeColor( pxColor(0xFF0000FF) );// RED
      c = canvas2d->strokeColor();
      canvas2d->fill(); //FILL

      EXPECT_TRUE (c.u == 0xFF0000FF);
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      // Gray Fill
      canvas2d->setFillColor(128, 128);// 50% gray
      c = canvas2d->fillColor();
      canvas2d->fill(); //FILL

      EXPECT_TRUE (c.r == 128);
      EXPECT_TRUE (c.g == 128);
      EXPECT_TRUE (c.b == 128);
      EXPECT_TRUE (c.a == 128);
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      // Gray Stroke
      canvas2d->setStrokeColor(128, 128);// 50% gray
      c = canvas2d->strokeColor();
      canvas2d->stroke(); //STROKE

      EXPECT_TRUE (c.r == 128);
      EXPECT_TRUE (c.g == 128);
      EXPECT_TRUE (c.b == 128);
      EXPECT_TRUE (c.a == 128);
  }

  void rasterTextureMatrixTest()
  {
      pxOffscreen texture;
      texture.initWithColor(256, 256, pxRed); // RED

      pxOffscreen offscreen;

      offscreen.init(256, 256);
      canvas2d->initWithBuffer(&offscreen); 

      canvas2d->setAlpha(1.0);
      canvas2d->setAlphaTexture(true);
      canvas2d->setTexture(&texture);
      canvas2d->setStrokeWidth(4.0);

      pxMatrix4T<float> m; // identity
      m.translate(2,2);
      canvas2d->setTextureMatrix(m); // SET
      m.translate(12,2);
      canvas2d->setMatrix(m); // SET

      canvas2d->roundRect(0,0, 100,100, 10, 10);
      canvas2d->rectangle(50,50, 20,20);
      canvas2d->fill(); // FILL

      canvas2d->setStrokeType(pxCanvas2d::StrokeType::inside);
      canvas2d->stroke(); //STROKE
      // How to test... read back pixels ?
  }

  void rasterTextureIdentityTest()
  {
      pxOffscreen texture;
      texture.initWithColor(256, 256, pxRed); // RED

      pxOffscreen offscreen;

      offscreen.init(256, 256);
      canvas2d->initWithBuffer(&offscreen); 

      canvas2d->setAlpha(0.9);

      float a =  canvas2d->alpha();
      EXPECT_TRUE (a == 0.9f);

      canvas2d->setAlphaTexture(true);
      canvas2d->setTexture(&texture);

      // FILL
      canvas2d->setFillColor(pxRed);
      canvas2d->roundRect(0,0, 100,100, 10, 10);
      canvas2d->rectangle(50,50, 20,20);
      canvas2d->fill();

      //STROKE
      canvas2d->setStrokeColor(128, 128);// 50% gray
      canvas2d->setStrokeType(pxCanvas2d::StrokeType::inside);
      canvas2d->setStrokeWidth(4.0);
      canvas2d->stroke();

      canvas2d->setTexture(NULL); // remove texture

      canvas2d->roundRect(0,0, 100,100, 10, 10);
      canvas2d->rectangle(50,50, 20,20);

      canvas2d->fill();
      canvas2d->stroke(); //STROKE

      canvas2d->clear();

      // How to test... read back pixels ?
  }

  // - -  - -  - -  - -  - -  - -  - -  - -  - -  - - 
  private:
    pxCanvas2d *canvas2d;
};


TEST_F(pxCanvas2dTest, pxCanvas2dTests)
{
  matrixTest();
  textureTest();
  textureClampTest();
  textureClampColorTest();
  biLerpTest();
  alphaTextureTest();
  overdrawTest();
  roundRectTest();
  bufferInitTest();
  getPenTest();
  testStrokeType();
  testColor();
  rasterTextureMatrixTest();
  rasterTextureIdentityTest();
}

