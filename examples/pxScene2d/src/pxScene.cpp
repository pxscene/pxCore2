// pxCore Copyright 2007-2015 John Robinson
// main.cpp

#include "pxCore.h"
#include "pxTimer.h"
#include "pxEventLoop.h"
#include "pxWindow.h"

#define ANIMATION_ROTATE_XYZ
#include "pxContext.h"
#include "pxScene2d.h"
#include "rtUrlUtils.h"

#include "rtNode.h"

extern rtNode script;

#include "jsbindings/rtWrapperUtils.h"

#if 0
#define ENTERSCENELOCK() rtWrapperSceneUpdateEnter();
#define EXITSCENELOCK()  rtWrapperSceneUpdateExit();
#else
#define ENTERSCENELOCK()
#define EXITSCENELOCK()
#endif

#ifndef PX_SCENE_VERSION
#define PX_SCENE_VERSION dev
#endif


class rtPromise; //fwd

pxEventLoop  eventLoop;
pxEventLoop* gLoop = &eventLoop;

pxContext context;

class sceneWindow : public pxWindow, public pxIViewContainer
{
public:
  sceneWindow(): mWidth(-1),mHeight(-1) {}
  virtual ~sceneWindow() {}

  void init(int x, int y, int w, int h, const char* url = NULL)
  {
    pxWindow::init(x,y,w,h);
    
    char buffer[1024];
    sprintf(buffer,"shell.js?url=%s",rtUrlEncodeParameters(url).cString());
    setView(new pxScriptView(buffer,"javascript/node/v8"));
  }

  rtError setView(pxIView* v)
  {
    ENTERSCENELOCK()
    mView = v;

    if (v)
    {
      v->setViewContainer(this);
      onSize(mWidth, mHeight);
    }
    EXITSCENELOCK()

    return RT_OK;
  }

  virtual void invalidateRect(pxRect* r)
  {
    pxWindow::invalidateRect(r);
  }

protected:

  virtual void onSize(int32_t w, int32_t h)
  {
//    if (mWidth != w || mHeight != h)
    {
      mWidth  = w;
      mHeight = h;
      ENTERSCENELOCK();
      if (mView)
        mView->onSize(w, h);
      EXITSCENELOCK();
    }
  }

  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onMouseDown(x, y, flags);
    EXITSCENELOCK();
  }

  virtual void onCloseRequest() 
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onCloseRequest();
    EXITSCENELOCK();
    // delete mView;

   // pxScene.cpp:104:12: warning: deleting object of abstract class type ‘pxIView’ which has non-virtual destructor will cause undefined behaviour [-Wdelete-non-virtual-dtor]

    mView = NULL;
    script.pump();
    script.garbageCollect();
    eventLoop.exit();
  }

  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags)
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onMouseUp(x, y, flags);
    EXITSCENELOCK();
  }

  virtual void onMouseLeave()
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onMouseLeave();
    EXITSCENELOCK();
  }

  virtual void onMouseMove(int32_t x, int32_t y)
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onMouseMove(x, y);
    EXITSCENELOCK();
  }

  virtual void onFocus()
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onFocus();
    EXITSCENELOCK();
  }
  virtual void onBlur()
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onBlur();
    EXITSCENELOCK();
  }

  virtual void onKeyDown(uint32_t keycode, uint32_t flags)
  {
    ENTERSCENELOCK();
    if (mView)
    {
      mView->onKeyDown(keycode, flags);
    }
    EXITSCENELOCK();
  }

  virtual void onKeyUp(uint32_t keycode, uint32_t flags)
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onKeyUp(keycode, flags);
    EXITSCENELOCK();
  }

  virtual void onChar(uint32_t c)
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onChar(c);
    EXITSCENELOCK();
  }

  virtual void onDraw(pxSurfaceNative )
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onDraw();
    EXITSCENELOCK();
  }

  virtual void onAnimationTimer()
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onUpdate(pxSeconds());
    EXITSCENELOCK();
    script.pump();
  }

  int mWidth;
  int mHeight;
  rtRefT<pxIView> mView;
};

#define xstr(s) str(s)
#define str(s) #s

int pxMain(int argc, char* argv[])
{
  char buffer[256];
  sprintf(buffer, "pxscene: %s", xstr(PX_SCENE_VERSION));
  sceneWindow win;
  // OSX likes to pass us some weird parameter on first launch after internet install
  win.init(10, 10, 1280, 720, (argc >= 2 && argv[1][0] != '-')?argv[1]:"browser.js");
  win.setTitle(buffer);
  // JRJR TODO Why aren't these necessary for glut... pxCore bug
  win.setVisibility(true);
  win.setAnimationFPS(60);

  #if 0
  sceneWindow win2;
  win2.init(50, 50, 1280, 720);
  #endif

// JRJR TODO this needs happen after GL initialization which right now only happens after a pxWindow has been created.
// Likely will move this to pxWindow...  as an option... a "context" type
// would like to decouple it from pxScene2d specifically
  context.init();

  eventLoop.run();

  return 0;
}
