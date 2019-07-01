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

#include "pxShaderResource.h"

#include "test_includes.h" // Needs to be included last

class pxShaderExample: public rtShaderResource
{
public:

};

//pxShaderManager

class pxShaderTest : public testing::Test
{
public:
    virtual void SetUp()
    {
      shaderEx = new pxShaderExample();
    }

    virtual void TearDown()
    {
      shaderEx = NULL;
    }

    void testInit()
    {
      pxError res = shaderEx->initShader(NULL, NULL);
      EXPECT_TRUE (res == PX_FAIL);
    }


    void testUniformLocation()
    {
      int res = shaderEx->getUniformLocation("NonExistant");
      EXPECT_TRUE (res == -1);
    }

    void testFindKey()
    {
      rtObjectRef       ao = new rtArrayObject;
      rtArrayObject *array = (rtArrayObject *) ao.getPtr();

      array->pushBack(rtString("one"));
      array->pushBack(rtString("two"));
      array->pushBack(rtString("three"));
      array->pushBack(rtString("myKey"));

      pxError res1 = findKey(array, "NonExistant");
      EXPECT_TRUE (res1 == PX_FAIL);

      pxError res2 = findKey(array, "myKey");
      EXPECT_TRUE (res2 == PX_OK);
    }


    void testApplyConfig() // RT object to Shader
    {
      rtObjectRef       ao = new rtArrayObject;
      rtArrayObject *array = (rtArrayObject *) ao.getPtr();

      pxObject myObj(NULL);

      pxError res1 = applyConfig(array, myObj);
      EXPECT_TRUE (res1 == PX_FAIL);
    }


    void testSetShaderConfig() // JS object to Shader
    {
      rtObjectRef       ao = new rtArrayObject;
      rtArrayObject *array = (rtArrayObject *) ao.getPtr();

      pxObject myObj(NULL);

      pxError res1 = setShaderConfig(array, myObj);
      EXPECT_TRUE (res1 == PX_FAIL);
    }

    void testUniforms()
    {
      rtObjectRef myObj( new pxObject(NULL) );

      pxError res = shaderEx->uniforms(myObj); // returns PX_OK - always
      EXPECT_TRUE (res == PX_OK);
    }

    void testDefaults()
    {
      EXPECT_TRUE ( shaderEx->isRealTime() == false );
      EXPECT_TRUE ( shaderEx->needsFbo()   == false );
      EXPECT_TRUE ( shaderEx->passes()     == 1);
    }

    void testUniformType()
    {
      rtString willFail("foo");
      EXPECT_TRUE ( shaderEx->getUniformType( willFail ) == UniformType_Unknown );
    }

    void testSetUniformVal()
    {
      rtString willFail("foo");
      EXPECT_TRUE ( shaderEx->setUniformVal( willFail , 0) == PX_FAIL );
    }

    void testSetUniformVals()
    {
      rtString junk("foo");
      EXPECT_TRUE ( shaderEx->setUniformVals( junk ) == PX_FAIL );
    }

    void testGLexception()
    {
      glException foo("test message");

      EXPECT_TRUE ( foo.desc() != NULL );
    }

    void testInit2() // trivial NULL
    {
      shaderProgram shader;
      pxError res = shader.initShader(NULL, NULL);

      EXPECT_TRUE (res == PX_FAIL);
    }

    void testDraw() // trivial NULL
    {
      shaderProgram shader;
      pxError res = shader.draw( 1, 1, NULL, 0.0, NULL, 0, NULL, NULL, 0);

      EXPECT_TRUE (res == PX_FAIL);
    }

    void testFillVectors() // trivial NULL
    {
      rtShaderResource shaderRes;

      pxError res1 = shaderRes.fillInt32Vec( NULL, NULL);
      EXPECT_TRUE (res1 == PX_FAIL);

      pxError res2 = shaderRes.fillFloatVec( NULL, NULL);
      EXPECT_TRUE (res2 == PX_FAIL);
    }

    void testBindTextures() // trivial NULL
    {
      rtShaderResource shaderRes;
      uniformLoc_t p;

      p.value = rtValue( (void *) NULL);

      pxError res1 = shaderRes.bindTexture3( p );
      EXPECT_TRUE (res1 == PX_FAIL);

      pxError res2 = shaderRes.bindTexture4( p );
      EXPECT_TRUE (res2 == PX_FAIL);

      pxError res3 = shaderRes.bindTexture5( p );
      EXPECT_TRUE (res3 == PX_FAIL);
    }

    void testSetScalars() // trivial NULL
    {
      rtShaderResource shaderRes;
      uniformLoc_t p;

      p.value = rtValue( (int) 0);

      pxError resINT = shaderRes.setUniform1i( p );
      EXPECT_TRUE (resINT == PX_OK);

      p.value = rtValue( (float) 0.0f);

      pxError resFLOAT = shaderRes.setUniform1i( p );
      EXPECT_TRUE (resFLOAT == PX_OK);
    }


    void testSetVectors()
    {
      rtShaderResource shaderRes;
      uniformLoc_t p;

      p.value = rtValue( (void *) NULL );

      pxError resINT2 = shaderRes.setUniform2iv( p );
      EXPECT_TRUE (resINT2 == PX_FAIL);

      pxError resFLOAT2 = shaderRes.setUniform2fv( p );
      EXPECT_TRUE (resFLOAT2 == PX_FAIL);


      pxError resINT3 = shaderRes.setUniform3iv( p );
      EXPECT_TRUE (resINT3 == PX_FAIL);

      pxError resFLOAT3 = shaderRes.setUniform3fv( p );
      EXPECT_TRUE (resFLOAT3 == PX_FAIL);


      pxError resINT4 = shaderRes.setUniform4iv( p );
      EXPECT_TRUE (resINT4 == PX_FAIL);

      pxError resFLOAT4 = shaderRes.setUniform4fv( p );
      EXPECT_TRUE (resFLOAT4 == PX_FAIL);
    }


    void testSetVectors2()
    {
      rtShaderResource shaderRes;
      uniformLoc_t p;

      rtObjectRef       ao = new rtArrayObject;
      rtArrayObject *array = (rtArrayObject *) ao.getPtr();

      array->pushBack((int) 1 );
      array->pushBack((int) 2 );
      array->pushBack((int) 3 );
      array->pushBack((int) 4 );

      p.value = rtValue( array );

      pxError resINT2 = shaderRes.setUniform2iv( p );
      EXPECT_TRUE (resINT2 == PX_OK);

      pxError resFLOAT2 = shaderRes.setUniform2fv( p );
      EXPECT_TRUE (resFLOAT2 == PX_OK);


      pxError resINT3 = shaderRes.setUniform3iv( p );
      EXPECT_TRUE (resINT3 == PX_OK);

      pxError resFLOAT3 = shaderRes.setUniform3fv( p );
      EXPECT_TRUE (resFLOAT3 == PX_OK);


      pxError resINT4 = shaderRes.setUniform4iv( p );
      EXPECT_TRUE (resINT4 == PX_OK);

      pxError resFLOAT4 = shaderRes.setUniform4fv( p );
      EXPECT_TRUE (resFLOAT4 == PX_OK);
    }

  private:
    pxShaderResourceRef  shaderEx;
};


TEST_F(pxShaderTest, pxShaderTests)
{
  testInit();
  testInit2();

  testUniformLocation();
  testFindKey();
  testApplyConfig();
  testSetShaderConfig();
  testUniforms();
  testDefaults();
  testUniformType();
  testSetUniformVal();
  testSetUniformVals();
  testGLexception();
  testDraw();
  testFillVectors();
  testBindTextures();
  testSetScalars();
  testSetVectors();
  testSetVectors2();
}

/*
#include "pxTexture.h"

#include "test_includes.h" // Needs to be included last

class pxTextureExample :public pxTexture
{
public:
    pxTextureExample() : pxTexture(), mWidth(0), mHeight(0) {}

    virtual pxError bindGLTexture(int / * tLoc * /){ return PX_OK; }
    virtual pxError bindGLTextureAsMask(int / * mLoc * /){ return PX_OK;}

    virtual pxError bindTexture() { return pxTexture::bindTexture(); }
    virtual pxError bindTextureAsMask() { return pxTexture::bindTextureAsMask(); }
    virtual pxError createTexture(pxOffscreen& o) { return pxTexture::createTexture(o); }
    virtual pxError deleteTexture(){ return PX_OK; }
    virtual int width() { return mWidth;}
    virtual int height() { return mHeight;}
    virtual pxError resizeTexture(int w, int h) { pxTexture::resizeTexture(w,h); mWidth = w; mHeight = h; return PX_OK; }
    virtual pxError getOffscreen(pxOffscreen&) { return PX_OK; }
    virtual unsigned int getNativeId() { return pxTexture::getNativeId(); }
    pxTextureType getType() { return mTextureType; }
    virtual pxError prepareForRendering() { return pxTexture::prepareForRendering(); }
    virtual pxError unloadTextureData() { return pxTexture::unloadTextureData(); }
    virtual pxError freeOffscreenData() { return pxTexture::freeOffscreenData(); }

private:
    int mWidth;
    int mHeight;

};

class pxTextureTest : public testing::Test
{
public:
    virtual void SetUp()
    {
      texture = new pxTextureExample();
    }

    virtual void TearDown()
    {
      texture = NULL;
    }

  void bindTest()
  {
    pxError bindGLTextureResult = texture->bindGLTexture(0);
    pxError bindGLTextureAsMaskResult = texture->bindGLTextureAsMask(0);
    pxError bindTextureResult = texture->bindTexture();
    pxError bindTextureAsMaskResult = texture->bindTextureAsMask();

    EXPECT_TRUE (bindGLTextureResult == PX_OK);
    EXPECT_TRUE (bindGLTextureAsMaskResult == PX_OK);
    EXPECT_TRUE (bindTextureResult == PX_FAIL);
    EXPECT_TRUE (bindTextureAsMaskResult == PX_FAIL);
  }

  void createTextureTest()
  {
    pxOffscreen o;
    pxError result = texture->createTexture(o);
    EXPECT_TRUE (result == PX_FAIL);
  }

  void deleteTextureTest()
  {
    pxError result = texture->deleteTexture();
    EXPECT_TRUE (result == PX_OK);
  }

  void widthTest()
  {
    texture->resizeTexture(5,7);
    int w = texture->width();
    EXPECT_TRUE (w == 5);
  }

  void heightTest()
  {
    texture->resizeTexture(5,7);
    int h = texture->height();
    EXPECT_TRUE (h == 7);
  }

  void getOffscreenTest()
  {
    pxOffscreen o;
    pxError result = texture->getOffscreen(o);
    EXPECT_TRUE (result == PX_OK);
  }

  void getNativeIdTest()
  {
    unsigned int id = texture->getNativeId();
    EXPECT_TRUE (id == 0);
  }

  void getTypeTest()
  {
    pxTextureType type = texture->getType();
    EXPECT_TRUE (type == PX_TEXTURE_UNKNOWN);
  }

  void prepareForRenderingTest()
  {
    pxError result = texture->prepareForRendering();
    EXPECT_TRUE (result == PX_OK);
  }

  void unloadTextureDataTest()
  {
    pxError result = texture->unloadTextureData();
    EXPECT_TRUE (result == PX_OK);
  }

  void freeOffscreenDataTest()
  {
    pxError result = texture->freeOffscreenData();
    EXPECT_TRUE (result == PX_OK);
  }

  void premultipliedAlphaTest()
  {
    texture->enablePremultipliedAlpha(true);
    bool result = texture->premultipliedAlpha();
    EXPECT_TRUE (result == true);
  }

  void getSurfaceTest()
  {
    void* surface = texture->getSurface();
    EXPECT_TRUE (surface == NULL);
  }

  void lastRenderTickTest()
  {
    texture->setLastRenderTick(12345);
    uint32_t tick = texture->lastRenderTick();
    EXPECT_TRUE (tick == 12345);
  }

  void downscaleSmoothTest()
  {
    texture->setDownscaleSmooth(true);
    bool result = texture->downscaleSmooth();
    EXPECT_TRUE (result == true);
  }
  private:
    pxTextureRef texture;
};

TEST_F(pxTextureTest, pxTextureTests)
{

  bindTest();
  createTextureTest();
  deleteTextureTest();
  widthTest();
  heightTest();
  getOffscreenTest();
  getNativeIdTest();
  getTypeTest();
  prepareForRenderingTest();
  unloadTextureDataTest();
  freeOffscreenDataTest();
  premultipliedAlphaTest();
  getSurfaceTest();
  lastRenderTickTest();
  downscaleSmoothTest();
}

*/