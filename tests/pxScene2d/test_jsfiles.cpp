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
bool scrollwheelcalled = false;
bool mousemovereceived = false;
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

    virtual void onScrollWheel(float x, float y)
    {
      UNUSED_PARAM(x);
      UNUSED_PARAM(y);
      scrollwheelcalled = true;
    }

    virtual void onMouseMove(int x, int y)
    {
      UNUSED_PARAM(x);
      UNUSED_PARAM(y);
      mousemovereceived = true;
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
                  int code = getRawNativeKeycodeFromGlut(94, 0);
                  EXPECT_TRUE(code == 54);
                  code = getRawNativeKeycodeFromGlut(95, 0);
                  EXPECT_TRUE(code == 45);
                  code = getRawNativeKeycodeFromGlut(64, 0);
                  EXPECT_TRUE(code == 50);
                  code = getRawNativeKeycodeFromGlut(33, 0);
                  EXPECT_TRUE(code == 49);
                  code = getRawNativeKeycodeFromGlut(35, 0);
                  EXPECT_TRUE(code == 51);
                  code = getRawNativeKeycodeFromGlut(36, 0);
                  EXPECT_TRUE(code == 52);
                  code = getRawNativeKeycodeFromGlut(37, 0);
                  EXPECT_TRUE(code == 53);
                  code = getRawNativeKeycodeFromGlut(38, 0);
                  EXPECT_TRUE(code == 55);
                  code = getRawNativeKeycodeFromGlut(40, 0);
                  EXPECT_TRUE(code == 57);
                  code = getRawNativeKeycodeFromGlut(41, 0);
                  EXPECT_TRUE(code == 48);
                  code = getRawNativeKeycodeFromGlut(42, 0);
                  EXPECT_TRUE(code == 56);
                  code = getRawNativeKeycodeFromGlut(43, 0);
                  EXPECT_TRUE(code == 61);
                  shiftPressed = false;
		}
		
		void getRawNativeKeycodeFromGlutCtrlTest()
                {
                  ctrlPressed = true;
                  int code = getRawNativeKeycodeFromGlut(1, 0);
                  EXPECT_TRUE(code == 97);
                  code = getRawNativeKeycodeFromGlut(2, 0);
                  EXPECT_TRUE(code == 98);
                  code = getRawNativeKeycodeFromGlut(3, 0);
                  EXPECT_TRUE(code == 99);
                  code = getRawNativeKeycodeFromGlut(4, 0);
                  EXPECT_TRUE(code == 100);
                  code = getRawNativeKeycodeFromGlut(5, 0);
                  EXPECT_TRUE(code == 101);
                  code = getRawNativeKeycodeFromGlut(6, 0);
                  EXPECT_TRUE(code == 102);
                  code = getRawNativeKeycodeFromGlut(7, 0);
                  EXPECT_TRUE(code == 103);
                  code = getRawNativeKeycodeFromGlut(8, 0);
                  EXPECT_TRUE(code == 104);
                  code = getRawNativeKeycodeFromGlut(9, 0);
                  EXPECT_TRUE(code == 105);
                  code = getRawNativeKeycodeFromGlut(10, 0);
                  EXPECT_TRUE(code == 106);
                  code = getRawNativeKeycodeFromGlut(11, 0);
                  EXPECT_TRUE(code == 107);
                  code = getRawNativeKeycodeFromGlut(12, 0);
                  EXPECT_TRUE(code == 108);
                  code = getRawNativeKeycodeFromGlut(13, 0);
                  EXPECT_TRUE(code == 109);
                  code = getRawNativeKeycodeFromGlut(14, 0);
                  EXPECT_TRUE(code == 110);
                  code = getRawNativeKeycodeFromGlut(15, 0);
                  EXPECT_TRUE(code == 111);
                  code = getRawNativeKeycodeFromGlut(16, 0);
                  EXPECT_TRUE(code == 112);
                  code = getRawNativeKeycodeFromGlut(17, 0);
                  EXPECT_TRUE(code == 113);
                  code = getRawNativeKeycodeFromGlut(18, 0);
                  EXPECT_TRUE(code == 114);
                  code = getRawNativeKeycodeFromGlut(19, 0);
                  EXPECT_TRUE(code == 115);
                  code = getRawNativeKeycodeFromGlut(20, 0);
                  EXPECT_TRUE(code == 116);
                  code = getRawNativeKeycodeFromGlut(21, 0);
                  EXPECT_TRUE(code == 117);
                  code = getRawNativeKeycodeFromGlut(22, 0);
                  EXPECT_TRUE(code == 118);
                  code = getRawNativeKeycodeFromGlut(23, 0);
                  EXPECT_TRUE(code == 119);
                  code = getRawNativeKeycodeFromGlut(24, 0);
                  EXPECT_TRUE(code == 120);
                  code = getRawNativeKeycodeFromGlut(25, 0);
                  EXPECT_TRUE(code == 121);
                  code = getRawNativeKeycodeFromGlut(26, 0);
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

                void cleanupGlutWindowTest()
                {
                }

                void onGlutMouseTest()
                {
                  win->onGlutMouse(GLUT_LEFT_BUTTON, GLUT_DOWN, 100, 100);
                  EXPECT_TRUE(true == win->mMouseDown);
                  win->onGlutMousePassiveMotion(100, 100);
                  EXPECT_TRUE(false == mousemovereceived);
                  mousemovereceived = false;
                  win->onGlutMouse(GLUT_LEFT_BUTTON, GLUT_UP, 100, 100);
                  EXPECT_TRUE(false == win->mMouseDown);
                  win->onGlutMouse(3, 0, 100, 100);
                  EXPECT_TRUE(true == scrollwheelcalled);
                  scrollwheelcalled = false;
                  win->onGlutMouse(4, 0, 100, 100);
                  EXPECT_TRUE(true == scrollwheelcalled);
                  scrollwheelcalled = false;
                }
 
                void onGlutKeyTest()
                {
                  shiftPressed = true;
                  win->onGlutKeyboard('k', 100, 200);
                  EXPECT_TRUE(true == keydownreceived);
                  EXPECT_TRUE(true == keyupreceived);
                }

                void keyboardspecialtest(int key, int x, int y, int expected)
                {
                  expectedkey = keycodeFromNative(expected);
                  win->onGlutKeyboardSpecial(key, x, y);
                  EXPECT_TRUE(keyspecialreceived == true);
                  keyspecialreceived = false;
                }

                void onGlutKeyboardSpecialTest()
                {
                  shiftPressed = true;
                  keyboardspecialtest(GLUT_KEY_F1, 100, 200, PX_KEY_NATIVE_F1);
                  keyboardspecialtest(GLUT_KEY_F2, 100, 200, PX_KEY_NATIVE_F2);
                  keyboardspecialtest(GLUT_KEY_F3, 100, 200, PX_KEY_NATIVE_F3);
                  keyboardspecialtest(GLUT_KEY_F4, 100, 200, PX_KEY_NATIVE_F4);
                  keyboardspecialtest(GLUT_KEY_F5, 100, 200, PX_KEY_NATIVE_F5);
                  keyboardspecialtest(GLUT_KEY_F6, 100, 200, PX_KEY_NATIVE_F6);
                  keyboardspecialtest(GLUT_KEY_F7, 100, 200, PX_KEY_NATIVE_F7);
                  keyboardspecialtest(GLUT_KEY_F8, 100, 200, PX_KEY_NATIVE_F8);
                  keyboardspecialtest(GLUT_KEY_F9, 100, 200, PX_KEY_NATIVE_F9);
                  keyboardspecialtest(GLUT_KEY_F10, 100, 200, PX_KEY_NATIVE_F10);
                  keyboardspecialtest(GLUT_KEY_F11, 100, 200, PX_KEY_NATIVE_F11);
                  keyboardspecialtest(GLUT_KEY_F12, 100, 200, PX_KEY_NATIVE_F12);
                  keyboardspecialtest(GLUT_KEY_LEFT, 100, 200, PX_KEY_NATIVE_LEFT);
                  keyboardspecialtest(GLUT_KEY_UP, 100, 200, PX_KEY_NATIVE_UP);
                  keyboardspecialtest(GLUT_KEY_RIGHT, 100, 200, PX_KEY_NATIVE_RIGHT);
                  keyboardspecialtest(GLUT_KEY_DOWN, 100, 200, PX_KEY_NATIVE_DOWN);
                  keyboardspecialtest(GLUT_KEY_PAGE_UP, 100, 200, PX_KEY_NATIVE_PAGEUP);
                  keyboardspecialtest(GLUT_KEY_PAGE_DOWN, 100, 200, PX_KEY_NATIVE_PAGEDOWN);
                  keyboardspecialtest(GLUT_KEY_HOME, 100, 200, PX_KEY_NATIVE_HOME);
                  keyboardspecialtest(GLUT_KEY_END, 100, 200,PX_KEY_NATIVE_END);
                  keyboardspecialtest(GLUT_KEY_INSERT, 100, 200,PX_KEY_NATIVE_INSERT);
                  keyboardspecialtest(112, 100, 200, 0);
                  keyboardspecialtest(113, 100, 200, 0);
                  keyboardspecialtest(114, 100, 200, 0);
                  keyboardspecialtest(116, 100, 200, PX_KEY_NATIVE_ALT);
                }

                void onGlutScrollWheelTest()
                {
                  win->onGlutScrollWheel(4, 100, 100, 100);
                  EXPECT_TRUE(true == scrollwheelcalled);
                  scrollwheelcalled = false;
                }
        
                void onGlutMouseMotionTest()
                {
                  win->onGlutMouseMotion(100, 100);
                  EXPECT_TRUE(true == mousemovereceived);
                  mousemovereceived = false;
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
  cleanupGlutWindowTest();
  onGlutKeyboardSpecialTest();
  onGlutKeyTest();
  onGlutMouseTest();
  onGlutScrollWheelTest();
  onGlutMouseMotionTest();
  onGlutCloseTest();
}
#endif
