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
#include <pxCore.h>
#include <pxWindow.h>
#include <pxScene2d.h>
#include <string.h>
#include "rtObject.h"
#include <pxTexture.h>
#include <pxContext.h>
#include <rtRef.h>
#include <stdlib.h>

#include "test_includes.h" // Needs to be included last

class shaderProgram;
class solidShaderProgram;
extern solidShaderProgram*  gSolidShader;
extern std::vector<pxTexture*> textureList;
extern rtMutex textureListMutex;
pxError addToTextureList(pxTexture* texture);
pxError removeFromTextureList(pxTexture* texture);
pxError ejectNotRecentlyUsedTextureMemory(int64_t bytesNeeded, uint32_t maxAge=5);

using namespace std;

#include "test_includes.h" // Needs to be included last

class A:public pxTexture
{

};

void fgDeinitializeLocal( void )
{
  printf("fgdeinitialize called locally \n");
  fflush(stdout);
  _exit(2);
}

class sceneWindow : public pxWindow, public pxIViewContainer
{
  public:
    void init(int x, int y, int w, int h, const char* url = NULL)
    {
      UNUSED_PARAM(url);
      pxWindow::init(x,y,w,h);
    }

    virtual void invalidateRect(pxRect* r)
    {
      UNUSED_PARAM(r);
    }

    virtual void* RT_STDCALL getInterface(const char* /*t*/)
    {
      return NULL;
    }

    rtError setView(pxIView* v)
    {
      UNUSED_PARAM(v);
      return RT_OK;
    }

    virtual void onAnimationTimer()
    {
    }

};

class pxContextTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      mSceneWin = new sceneWindow();
      mSceneWin->init(0,0,1280,720);
      atexit(fgDeinitializeLocal);
      mContext.init();
    }

    virtual void TearDown()
    {
    }

    void showOutlinesTest()
    {
      mContext.setShowOutlines(true);
      EXPECT_TRUE (mContext.showOutlines() == true);
    }

    void initTwiceTest()
    {
      mContext.init();
      EXPECT_TRUE (NULL != gSolidShader);
    }

    void setSizeTest()
    {
      mContext.setSize(1280,720);
      int w,h;
      mContext.getSize(w,h);
      EXPECT_TRUE (w == 1280);
      EXPECT_TRUE (h == 720);
    }

    void enableInternalContextTest()
    {
      EXPECT_TRUE ( RT_OK == mContext.enableInternalContext(false));
    }

    void setEjectTextureAgeTest()
    {
      EXPECT_TRUE ( PX_OK == mContext.setEjectTextureAge(100));
      EXPECT_TRUE (mContext.mEjectTextureAge == 100);
    }

    void enableDirtyRectanglesTest()
    {
      mContext.enableDirtyRectangles(false);
      EXPECT_TRUE (glIsEnabled(GL_SCISSOR_TEST) == GL_FALSE);
      mContext.enableDirtyRectangles(true);
      EXPECT_TRUE (glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
    }

    void clearColorTest()
    {
      float fillColor[] = {0,0,0,0};
      mContext.clear(0,0,fillColor);
      EXPECT_TRUE (glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
    }

    void clearTest()
    {
      mContext.clear(0,0,720,720);
      EXPECT_TRUE (glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
    }

    void enableClipTest()
    {
      mContext.enableClipping(true);
      EXPECT_TRUE (glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
    }

    void disableClipTest()
    {
      mContext.enableClipping(false);
      EXPECT_TRUE (glIsEnabled(GL_SCISSOR_TEST) == GL_FALSE);
    }

    void updateFramebufferFailTest()
    {
      EXPECT_TRUE (mContext.updateFramebuffer(NULL, 0, 0) == PX_FAIL);
    }

    void pxTextureNoneTest()
    {
      pxTextureRef noneTexture = mContext.createTexture();
      EXPECT_TRUE (noneTexture->width() == 0);
      EXPECT_TRUE (noneTexture->height() == 0);
      EXPECT_TRUE (noneTexture->deleteTexture() == PX_FAIL);
      EXPECT_TRUE (noneTexture->resizeTexture(0,0) == PX_FAIL);
      pxOffscreen o;
      EXPECT_TRUE (noneTexture->getOffscreen(o) == PX_FAIL);
      EXPECT_TRUE (noneTexture->bindGLTexture(0) == PX_FAIL);
      EXPECT_TRUE (noneTexture->bindGLTextureAsMask(0) == PX_FAIL);
    }

    void isObjectOnScreenTest()
    {
      EXPECT_TRUE (mContext.isObjectOnScreen(0,0,0,0) == true);
    }

    void textureMemoryOverflowTrueTest()
    {
      char *buffer = new char[100*100];
      pxTextureRef alphaTexture = mContext.createTexture(100,100,100,100,buffer);
      EXPECT_TRUE (mContext.textureMemoryOverflow(alphaTexture) == 0);
      delete[] buffer;
    }

    void textureMemoryOverflowFalseTest()
    {
      int64_t oldLimit = mContext.mTextureMemoryLimitInBytes;
      mContext.setTextureMemoryLimit(0);
      char *buffer = new char[100*100];
      pxTextureRef alphaTexture = mContext.createTexture(100,100,100,100,buffer);
      EXPECT_TRUE (mContext.textureMemoryOverflow(alphaTexture) > 0);
      mContext.setTextureMemoryLimit(oldLimit);
      delete[] buffer;
    }

    void adjustCurrentTextureMemorySizeTest()
    {
      int64_t oldVal = mContext.currentTextureMemoryUsageInBytes();
      mContext.mCurrentTextureMemorySizeInBytes = 0;
      mContext.adjustCurrentTextureMemorySize(-100);
      EXPECT_TRUE (0 == mContext.currentTextureMemoryUsageInBytes());
      mContext.mCurrentTextureMemorySizeInBytes = oldVal;
    }

    void ejectTextureMemoryForceReject()
    {
      bool mEnableTextureMemoryMonitoringTemp = mContext.mEnableTextureMemoryMonitoring;
      mContext.mEnableTextureMemoryMonitoring = true;
      int64_t ret = mContext.ejectTextureMemory(0, true);
      EXPECT_TRUE (0 == ret);
      mContext.mEnableTextureMemoryMonitoring = mEnableTextureMemoryMonitoringTemp;
    }

    void ejectTextureMemoryNoForceReject()
    {
      bool mEnableTextureMemoryMonitoringTemp = mContext.mEnableTextureMemoryMonitoring;
      mContext.mEnableTextureMemoryMonitoring = true;
      int64_t ret = mContext.ejectTextureMemory(0, false);
      EXPECT_TRUE (0 == ret);
      mContext.mEnableTextureMemoryMonitoring = mEnableTextureMemoryMonitoringTemp;
    }

    void snapshotTest()
    {
      pxOffscreen offscreen;
      mContext.snapshot(offscreen);
      EXPECT_TRUE (offscreen.upsideDown() == true);
    }

    void getMatrixTest()
    {
      pxMatrix4f m;
      float* vals;
      m = mContext.getMatrix();
      vals = m.data();
      EXPECT_TRUE (vals[0] == 1.0);
      EXPECT_TRUE (vals[1] == 0.0);
      EXPECT_TRUE (vals[2] == 0.0);
      EXPECT_TRUE (vals[3] == 0.0);
    }

    void mapToScreenCoordinatesTest()
    {
      float inX = 2;
      float inY = 3;
      int outX;
      int outY;
      mContext.mapToScreenCoordinates(inX,inY,outX,outY);
      EXPECT_TRUE (outX == 2.0);
      EXPECT_TRUE (outY == 3.0);
    }

    void mapToScreenCoordinatesInMatrixTest()
    {
      float inX = 2;
      float inY = 3;
      int outX;
      int outY;
      pxMatrix4f m;
      m.identity();
      for (int i=0; i<16; i++)
        m.mValues[i] = 1.0;
      mContext.mapToScreenCoordinates(m, inX,inY,outX,outY);
      EXPECT_TRUE (outX == 1);
      EXPECT_TRUE (outY == 1);
    }

    void mapToScreenCoordinatesInMatrixZeroWidthTest()
    {
      float inX = 2;
      float inY = 3;
      int outX;
      int outY;
      pxMatrix4f m;
      m.identity();
      for (int i=0; i<16; i++)
        m.mValues[i] = 0.0;
      mContext.mapToScreenCoordinates(m, inX,inY,outX,outY);
      EXPECT_TRUE (outX == 0);
      EXPECT_TRUE (outY == 0);
    }

    void drawDiagRectSuccessTest()
    {
      mContext.setShowOutlines(true);
      float values[4] = {1.0,1.0,1.0,1.0};
      mContext.drawDiagRect(0,0,1280,720,values);
    }

    void drawRectTest()
    {
      float fillColor[4] = {1,1,1,1};
      float lineColor[4] = {1,1,1,1};
      mContext.drawRect(1024,720,10,fillColor,lineColor);
    }

    void drawDiagLineTest()
    {
      mContext.setShowOutlines(true);
      float values[4] = {1.0,1.0,1.0,1.0};
      mContext.drawDiagLine(0,0,1,1,values);
    }
   
    void drawImage9Test()
    {
      float gAlphaTemp = mContext.getAlpha();
      mContext.setAlpha(1.0);
      pxOffscreen mOffscreen;
      pxTextureRef mOffscreenTexture = mContext.createTexture(mOffscreen);
      mContext.drawImage9(1.0, 1.0, 1.0, 1.0, 1.0, 1.0 , mOffscreenTexture); 
      mContext.setAlpha(gAlphaTemp);
    }
   
    void drawImageTextureDimDefault()
    {
      pxOffscreen mOffscreen;
      pxTextureRef mOffscreenTexture = mContext.createTexture(mOffscreen);
      mContext.drawImage(1.0, 1.0, 1.0, 1.0, mOffscreenTexture, mOffscreenTexture, true); 
    }
    
    void drawImage9BorderTest()
    {
      float gAlphaTemp = mContext.getAlpha();
      mContext.setAlpha(1.0);
      pxOffscreen mOffscreen;
      pxTextureRef mOffscreenTexture = mContext.createTexture(mOffscreen);
      float blackColor[4] = {0.0, 0.0, 0.0, 1.0};
      mContext.drawImage9Border(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, true, blackColor,  mOffscreenTexture); 
      mContext.setAlpha(gAlphaTemp);
    }
   
    void isTextureSpaceAvailableTest()
    {
      bool mEnableTextureMemoryMonitoringTemp = mContext.mEnableTextureMemoryMonitoring;
      mContext.mEnableTextureMemoryMonitoring = true;
      pxOffscreen mOffscreen;
      pxTextureRef mOffscreenTexture = mContext.createTexture(mOffscreen);
      bool ret = mContext.isTextureSpaceAvailable(mOffscreenTexture, true);
      EXPECT_TRUE (true == ret);
      mContext.mEnableTextureMemoryMonitoring = mEnableTextureMemoryMonitoringTemp;
    }   


private:

    sceneWindow* mSceneWin;
    pxContext mContext;
};

TEST_F(pxContextTest, pxContextTests)
{
  showOutlinesTest();
  initTwiceTest();
  setSizeTest();
  enableInternalContextTest();
  setEjectTextureAgeTest();
  enableDirtyRectanglesTest();
  clearColorTest();
  clearTest();
  enableClipTest();
  disableClipTest();
  updateFramebufferFailTest();
  pxTextureNoneTest();
  isObjectOnScreenTest();
  textureMemoryOverflowTrueTest();
  textureMemoryOverflowFalseTest();
  adjustCurrentTextureMemorySizeTest();
  ejectTextureMemoryForceReject();
  ejectTextureMemoryNoForceReject();
  snapshotTest();
  getMatrixTest();
  mapToScreenCoordinatesTest();
  mapToScreenCoordinatesInMatrixTest();
  mapToScreenCoordinatesInMatrixZeroWidthTest();
  drawDiagRectSuccessTest();
  drawRectTest();
  drawDiagLineTest();
  drawImage9Test();
  drawImageTextureDimDefault();
  drawImage9BorderTest();
  isTextureSpaceAvailableTest();
}


void addToTextureTest()
{
  EXPECT_TRUE (addToTextureList(NULL) == RT_OK);
  std::vector<pxTexture*>::iterator it;
  textureListMutex.lock();
  it = find (textureList.begin(), textureList.end(), (pxTexture*)NULL);
  EXPECT_TRUE (it != textureList.end());
  textureListMutex.unlock();
}

void removeFromTextureListTest()
{
  EXPECT_TRUE (removeFromTextureList(NULL) == RT_OK);
  EXPECT_TRUE (removeFromTextureList((pxTexture*)0x9aabc) == RT_OK);
}

void ejectNotRecentlyUsedTextureMemoryTest()
{
  int64_t bytesNeeded = 4;
  EXPECT_TRUE (ejectNotRecentlyUsedTextureMemory(bytesNeeded) == PX_OK);
}


TEST(pxContextGLFileTest, pxContextGLFileTests)
{
  addToTextureTest();
  removeFromTextureListTest();
  ejectNotRecentlyUsedTextureMemoryTest();
}

class pxFBOTextureTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      mFramebuffer = mContext.createFramebuffer(1024,768,false);
      mContext.setFramebuffer(NULL);
      mContext.setFramebuffer(mFramebuffer);
    }

    virtual void TearDown()
    {
    }

    void resizeFboTextureTest()
    {
      EXPECT_TRUE (PX_OK == mContext.updateFramebuffer(mFramebuffer,1280,720));
      EXPECT_TRUE(mFramebuffer->getTexture()->width() == 1280);
      EXPECT_TRUE(mFramebuffer->getTexture()->height() == 720);
    }

    void getNativeIdTest()
    {
      EXPECT_TRUE (PX_OK == mContext.updateFramebuffer(mFramebuffer,1280,720));
      EXPECT_TRUE(mFramebuffer->getTexture()->getNativeId() != 0);
    }

    void bindGLTextureTest()
    {
      mFramebuffer->getTexture()->unloadTextureData();
      EXPECT_TRUE(mFramebuffer->getTexture()->bindGLTexture(0) == PX_OK);
      mFramebuffer->getTexture()->loadTextureData();
    }
 

    void bindGLTextureAsMaskSuccessTest()
    {
      EXPECT_TRUE(mFramebuffer->getTexture()->bindGLTextureAsMask(0) == PX_OK);
    }

    private:
      pxContext mContext;
      pxContextFramebufferRef mFramebuffer;
};

TEST_F(pxFBOTextureTest, pxFBOTextureTests)
{
  resizeFboTextureTest();
  getNativeIdTest();
  bindGLTextureTest();
  bindGLTextureAsMaskSuccessTest();
}

class pxTextureOffscreenTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      mOffscreenTexture = mContext.createTexture(mOffscreen);
    }

    virtual void TearDown()
    {
    }

    void bindGLTextureAsMaskTest()
    {
      EXPECT_TRUE (PX_OK == mOffscreenTexture->bindGLTextureAsMask(0));
    }
    
    void bindGLTextureTest()
    {
      EXPECT_TRUE (PX_OK == mOffscreenTexture->bindGLTexture(0));
    }
    
    void bindGLTextureUnloadTest()
    {
      mOffscreenTexture->unloadTextureData();      
      EXPECT_TRUE (PX_OK !=  mOffscreenTexture->bindGLTexture(0));
    }
  
    void prepareForRenderingTest()
    {
      EXPECT_TRUE (PX_OK == mOffscreenTexture->prepareForRendering());  
    }

    void loadTextureDataTest()
    {
      pxOffscreen mOffscreen;
      pxTextureRef mOffscreenTexture = mContext.createTexture(mOffscreen);
      EXPECT_TRUE (PX_OK == mOffscreenTexture->deleteTexture());
      EXPECT_TRUE (PX_OK == mOffscreenTexture->loadTextureData());
    }

    private:
      pxOffscreen mOffscreen;
      pxTextureRef mOffscreenTexture;
      pxContext mContext;
};

TEST_F(pxTextureOffscreenTest, pxTextureOffscreenTests)
{
  bindGLTextureAsMaskTest();
  bindGLTextureTest();
  bindGLTextureUnloadTest();
  prepareForRenderingTest();
  loadTextureDataTest();
}

class pxAlphaTextureTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      memset(buffer, 0, 10000);
      mAlphaTexture = mContext.createTexture(100,10,100,10,buffer);
    }

    virtual void TearDown()
    {
    }

    void createAlphaTextureFailureTest()
    {
      char temp[100];
      mContext.createTexture(100,10,0,0,temp);
    }

    void bindGLTextureAsMaskUninitTest()
    {
      EXPECT_TRUE (PX_NOTINITIALIZED == mAlphaTexture->bindGLTextureAsMask(0));
    }

    void bindGLTextureAsMaskTest()
    {
      EXPECT_TRUE (PX_OK == mAlphaTexture->bindGLTexture(0));
      EXPECT_TRUE (PX_OK == mAlphaTexture->bindGLTextureAsMask(0));
    }

    private:
      pxTextureRef mAlphaTexture;
      pxContext mContext;
      char buffer[10000];
};

TEST_F(pxAlphaTextureTest, pxAlphaTextureTests)
{
  createAlphaTextureFailureTest();
  bindGLTextureAsMaskUninitTest();
  bindGLTextureAsMaskTest();
}

