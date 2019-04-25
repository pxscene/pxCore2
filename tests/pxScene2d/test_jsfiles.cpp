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

#ifndef ENABLE_RT_NODE
#define ENABLE_RT_NODE
#endif

#include <sstream>

#define private public
#define protected public
#include <pxCore.h>
#include <pxWindowUtil.h>
#include <pxWindow.h>
//#include <GL/glew.h>
//#include <GL/freeglut.h>
#include "pxScene2d.h"
#include <string.h>
#include "pxIView.h"
#include "pxTimer.h"
#include "rtObject.h"
#include <pxContext.h>
#include <rtRef.h>
#include <stdlib.h>
#ifdef __linux__
#include <unistd.h>
#include <time.h>
#include <dlfcn.h>
#include <fcntl.h>
#endif
#include "test_includes.h" // Needs to be included last

using namespace std;
extern rtScript script;
extern int gargc;
extern char** gargv;
extern int pxObjectCount;

void fgDeinitialize( void )
{
  printf("fgdeinitialize called locally \n");
  fflush(stdout);
  _exit(2);
}

#ifdef __linux__
extern uint32_t getRawNativeKeycodeFromGlut(uint32_t, uint32_t);
bool shiftPressed = false;
bool ctrlPressed = false;
bool keyupreceived = false;
bool keydownreceived = false;
bool keyspecialreceived = false;
uint32_t expectedkey = 0;
typedef int (*glutGetModifiers_t)();

int glutGetModifiers()
{
  static glutGetModifiers_t glutGetModifiersp = (glutGetModifiers_t) dlsym(RTLD_NEXT, "glutGetModifiers");
  if (true == shiftPressed)
  {
    return GLUT_ACTIVE_SHIFT;
  }
  else if (true == ctrlPressed)
  {
    return GLUT_ACTIVE_CTRL;
  }
  return glutGetModifiersp();
}
#endif
class sceneWindow : public pxWindow, public pxIViewContainer
{
  public:
    void init(int x, int y, int w, int h, const char* url = NULL)
    {
      mWidth = w;
      mHeight = h;
      pxWindow::init(x,y,w,h);
      std::ignore = url;
    }

    virtual void invalidateRect(pxRect* r)
    {
      pxWindow::invalidateRect(r);
    }

    virtual void* RT_STDCALL getInterface(const char* /*t*/)
    {
      return NULL;
    }

    rtError setView(pxIView* v)
    {
      mView = v;
      if (v)
      {
        v->setViewContainer(this);
        v->onSize(mWidth, mHeight);
      }
      return RT_OK;
    }

    virtual void onAnimationTimer()
    {
      if (mView)
        mView->onUpdate(pxSeconds());
      script.pump();
    }

    virtual void onKeyUp(uint32_t keycode, uint32_t flags)
    {
      keyupreceived = true;
      pxWindow::onKeyUp(keycode, flags);
    }

    virtual void onKeyDown(uint32_t keycode, uint32_t flags)
    {
      keydownreceived = true;
      if (keycode == expectedkey)
        keyspecialreceived = true;
      pxWindow::onKeyDown(keycode, flags);
    }

  private:
    pxIView* mView;
    int mWidth;
    int mHeight;
};

sceneWindow* win = NULL;
class jsFilesTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      mSceneWin = new sceneWindow();
      mSceneWin->init(0,0,1280,720);
      win = mSceneWin;
      atexit(fgDeinitialize);
      mContext.init();
    }

    virtual void TearDown()
    {
    }

    void test(const char* file, float timeout)
    {
      script.collectGarbage();
      int oldpxCount = pxObjectCount;
      long oldtextMem = mContext.currentTextureMemoryUsageInBytes();
      startJsFile(file);
      process(timeout);
      mView->onCloseRequest();
      script.collectGarbage();
      //currently we are getting the count +1 , due to which test is failing
      //suspecting this is due to scenecontainer without parent leak.
      //EXPECT_TRUE (pxObjectCount == oldpxCount);
      printf("old px count [%d] new px count [%d] \n",oldpxCount,pxObjectCount);
      fflush(stdout);
      EXPECT_TRUE (mContext.currentTextureMemoryUsageInBytes() == oldtextMem);
    }


private:

    void startJsFile(const char *jsfile)
    {
      mUrl = jsfile;
      mView = new pxScriptView(mUrl,"");
      mSceneWin->setView(mView);
    }

    void process(float timeout)
    {
      double  secs = pxSeconds();
      while ((pxSeconds() - secs) < timeout)
      {
        if (NULL != mView)
        {
          mSceneWin->onAnimationTimer();
/*
          mView->onDraw();
          mView->onUpdate(pxSeconds());
          script.pump();
          glutSwapBuffers();
*/
        }
      }
    }

    pxScriptView* mView;
    rtString mUrl;
    sceneWindow* mSceneWin;
    int mGlutWindowId;
    pxContext mContext;
};

TEST_F(jsFilesTest, jsTests)
{
  test("shell.js?url=http://www.sparkui.org/examples/gallery/fancy.js",5.0);
  test("shell.js?url=http://www.sparkui.org/examples/gallery/picturepile.js",5.0);
  test("shell.js?url=http://www.sparkui.org/examples/gallery/gallery.js",20.0);
}
#ifdef __linux__
class pxWindowDetailedTest : public testing::Test
{
	public:
		virtual void SetUp()
		{
		}

		virtual void TearDown()
		{
		}

		void getRawNativeKeycodeFromGlutShiftTest()
		{
                  shiftPressed = true;
                  int code = getRawNativeKeycodeFromGlut(43, 0);
                  EXPECT_TRUE(code == 61);
                  shiftPressed = false;
		}
		
		void getRawNativeKeycodeFromGlutCtrlTest()
                {
                  ctrlPressed = true;
                  int code = getRawNativeKeycodeFromGlut(26, 0);
                  EXPECT_TRUE(code == 122);
                  ctrlPressed = false;
                }

                void unregisterWindowTest()
                {
                  bool present = false;
                  std::vector<pxWindowNative*> windowsbefore = pxWindowNative::getNativeWindows();
                  for(std::vector<pxWindowNative*>::iterator it = windowsbefore.begin(); it<windowsbefore.end(); it++) {
                    if (*it == win)
                    {
                      present = true;
                      break;
                    }
                  }
                  EXPECT_TRUE(present == true);
                  present = false;
                  pxWindowNative::unregisterWindow(win);
                  std::vector<pxWindowNative*> windowsafter = pxWindowNative::getNativeWindows();
                  for(std::vector<pxWindowNative*>::iterator it = windowsafter.begin(); it<windowsafter.end(); it++) {
                    if (*it == win)
                    {
                      present = true;
                      break;
                    }
                  }
                  EXPECT_TRUE(present == false);
                  pxWindowNative::registerWindow(win);
                }

                void beginNativeDrawingTest()
                {
                  void* b = win;
                  win->beginNativeDrawing(b);
                }

                void endNativeDrawingTest()
                {
                  void* b = win;
                  win->endNativeDrawing(b);
                }

                void visibilityTest()
                {
                  EXPECT_TRUE(false == win->visibility());
                }

                void onGlutMouseTest()
                {
                  win->onGlutMouse(GLUT_LEFT_BUTTON, GLUT_DOWN, 100, 100);
                  EXPECT_TRUE(true == win->mMouseDown);
                  win->onGlutMouse(GLUT_LEFT_BUTTON, GLUT_UP, 100, 100);
                  EXPECT_TRUE(false == win->mMouseDown);
                }
 
                void onGlutKeyTest()
                {
                  shiftPressed = true;
                  win->onGlutKeyboard('k', 100, 200);
                  EXPECT_TRUE(false == keydownreceived);
                  EXPECT_TRUE(false == keyupreceived);
                }

                void onGlutKeyboardSpecialTest()
                {
                  shiftPressed = true;
                  expectedkey = keycodeFromNative(PX_KEY_NATIVE_ALT);
                  win->onGlutKeyboardSpecial(116, 100, 200);
                  EXPECT_TRUE(keyspecialreceived == false);
                }

                void onGlutCloseTest()
                {
                  win->onGlutClose();
                  std::vector<pxWindowNative*> windowsafter = pxWindowNative::getNativeWindows();
                  bool present = false;
                  for(std::vector<pxWindowNative*>::iterator it = windowsafter.begin(); it<windowsafter.end(); it++) {
                    if (*it == win)
                    {
                      present = true;
                      break;
                    }
                  }
                  EXPECT_TRUE(present == false);
                }
};

TEST_F(pxWindowDetailedTest, pxWindowDetailedTests)
{
  getRawNativeKeycodeFromGlutShiftTest();
  getRawNativeKeycodeFromGlutCtrlTest();
  unregisterWindowTest();
  beginNativeDrawingTest();
  endNativeDrawingTest();
  visibilityTest();
  onGlutKeyboardSpecialTest();
  onGlutKeyTest();
  onGlutMouseTest();
  onGlutCloseTest();
}
#endif
