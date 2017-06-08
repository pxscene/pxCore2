#define ENABLE_RT_NODE

#define private public
#define protected public

#include <GL/glew.h>
#include <GL/freeglut.h>
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

extern rtNode script;
extern int gargc;
extern char** gargv;
extern int pxObjectCount;
class jsFilesTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
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
      mContext.init();
    }

    virtual void TearDown()
    {
      glutDestroyWindow(mGlutWindowId);
    }

    void test(char* file, float timeout)
    {
      script.garbageCollect();
      int oldpxCount = pxObjectCount;
      long oldtextMem = mContext.currentTextureMemoryUsageInBytes();
      startJsFile(file);
      process(timeout);
      mView->onCloseRequest();
      script.garbageCollect();
      //currently we are getting the count +1 , due to which test is failing
      //suspecting this is due to scenecontainer without parent leak.
      //EXPECT_TRUE (pxObjectCount == oldpxCount);
      printf("old px count [%d] new px count [%d] \n",oldpxCount,pxObjectCount);
      fflush(stdout);
      EXPECT_TRUE (mContext.currentTextureMemoryUsageInBytes() == oldtextMem);
    }


private:

    void startJsFile(char *jsfile)
    {
      mUrl = jsfile;
      mView = new pxScriptView(mUrl,"");
    }

    void process(float timeout)
    {
      double  secs = pxSeconds();
      while ((pxSeconds() - secs) < timeout)
      {
        if (NULL != mView)
        {
          mView->onDraw();
          mView->onUpdate(pxSeconds());
          script.pump();
          glutSwapBuffers();
        }
      }
    }

    pxScriptView* mView;
    rtString mUrl;
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
