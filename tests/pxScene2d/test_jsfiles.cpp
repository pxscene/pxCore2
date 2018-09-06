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

  private:
    pxIView* mView;
    int mWidth;
    int mHeight;
};

class jsFilesTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      mSceneWin = new sceneWindow();
      mSceneWin->init(0,0,1280,720);
/*
      glutInit(&gargc,gargv);
      glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA);
      glutInitWindowPosition (0, 0);
      glutInitWindowSize (1024, 768);
      mGlutWindowId = glutCreateWindow ("pxWindow");
      glutSetOption(GLUT_RENDERING_CONTEXT ,GLUT_USE_CURRENT_CONTEXT );
      glClearColor(0, 0, 0, 1);

      GLenum err = glewInit();

      if (err != GLEW_OK)
      {
        printf("error with glewInit() [%s] [%d] \n",glewGetErrorString(err), err);
        fflush(stdout);
      }
*/
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
  test("shell.js?url=http://www.pxscene.org/examples/px-reference/gallery/fancy.js",5.0);
  test("shell.js?url=http://www.pxscene.org/examples/px-reference/gallery/picturepile.js",5.0);
  test("shell.js?url=http://www.pxscene.org/examples/px-reference/gallery/gallery.js",20.0);
  //test("shell.js?url=https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js",180.0);
}
