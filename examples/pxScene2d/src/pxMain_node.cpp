// pxCore Copyright 2007-2015 John Robinson
// main.cpp

#include "pxCore.h"
#include "pxTimer.h"
#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxScene2d.h"

#include "rtNode.h"

#include "jsbindings/rtWrapperUtils.h"

#define ENTERSCENELOCK() rtWrapperSceneUpdateEnter();
#define EXITSCENELOCK()  rtWrapperSceneUpdateExit(); 

rtNode script;
pxEventLoop  eventLoop;

rtError getScene(int /*numArgs*/, const rtValue* /*args*/, rtValue* result, void* ctx)
{
  if (result)
    *result = (pxScene2d*)ctx;; // return the scene reference

  return RT_OK;
}

class sceneWindow : public pxWindow, public pxIViewContainer
{
public:
  sceneWindow() {}
  virtual ~sceneWindow() {}

  void init(int x, int y, int w, int h, const char* uri = NULL)
  {
    pxWindow::init(x,y,w,h);

    pxScene2dRef scene = new pxScene2d;
    scene->init();
    
    setView(scene);

    ctx = script.createContext();
    ctx->add("getScene", new rtFunctionCallback(getScene, scene.getPtr()));

    char buffer[256];
    sprintf(buffer, "var pxArg_url=\"%s\";", uri?uri:"browser.js");
    ctx->runScript(buffer);
    ctx->runFile("start.js");
  }
  
  rtError setView(pxIView* v)
  {
    mView = v;

    if (v)
    {
      v->setViewContainer(this);
      onSize(mWidth,mHeight);
    }
      
    return RT_OK;
  }

  virtual void invalidateRect(pxRect* r)
  {
    pxWindow::invalidateRect(r);
  }

protected:

  virtual void onSize(int32_t w, int32_t h)
  {     
    mWidth  = w;
    mHeight = h;
    ENTERSCENELOCK();
    if (mView)
      mView->onSize(w, h);
    EXITSCENELOCK();
  }

  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    ENTERSCENELOCK();
    if (mView)
      mView->onMouseDown(x, y, flags);
    EXITSCENELOCK();
  }

  virtual void onCloseRequest() {}

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
  }

  int mWidth;
  int mHeight;
  rtRefT<pxIView> mView;

  rtNodeContextRef ctx;
};

int pxMain(int argc, char* argv[])
{
  sceneWindow win;
  win.init(10, 10, 1280, 720, (argc >= 2)?argv[1]:"browser.js");

  #if 0
  sceneWindow win2;
  win2.init(50, 50, 1280, 720);
  #endif
  
  eventLoop.run();
  
  return 0;
}
