#ifndef XXWINDOW_H
#define XXWINDOW_H


#include <stdio.h>

#include "pxCore.h"

#include <pxEventLoop.h>
#include <pxScene2d.h>
#include <pxWindow.h>
#include <pxWindowUtil.h>
#include <pxTimer.h>

#include "node.h"

//#define USE_UV_DUMMY_EVENTS

using namespace v8;
using namespace node;


extern pxEventLoop* gLoop;


#ifdef RUNINMAIN
#define ENTERSCENELOCK()
#define EXITSCENELOCK() 
#else
#define ENTERSCENELOCK() rtWrapperSceneUpdateEnter();
#define EXITSCENELOCK()  rtWrapperSceneUpdateExit(); 
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class xxWindow : public pxWindow, public pxIViewContainer
{
public:
  xxWindow(Isolate* isolate, int x, int y, int w, int h)
    : pxWindow()
    //, mEventLoop(new pxEventLoop())
  {
    UNUSED_PARAM(isolate);
    
    rtLogInfo("creating native with [%d, %d, %d, %d]", x, y, w, h);
    init(x, y, w, h);
  }
  
#ifdef USE_UV_DUMMY_EVENTS
  void startTimers()
  {
    rtLogInfo("starting background thread for event loop processing");
    startEventProcessingThread();

    // we start a timer in case there aren't any other evens to the keep the
    // nodejs event loop alive. Fire a time repeatedly.
    uv_timer_init(uv_default_loop(), &mTimer);
    
    // TODO experiment crank up the timers so we can pump cocoa messages on main thread
    #ifdef RUNINMAIN
    uv_timer_start(&mTimer, timerCallback, 0, 5);
    #else
    uv_timer_start(&mTimer, timerCallback, 1000, 1000);
    #endif
    setAnimationFPS(60);
  }

  Local<Object> scene(Isolate* isolate) const
  {    
    if (!mView)
    {
      // Lazy creation of scene
      xxWindow* this_ = const_cast<xxWindow*>(this);
      rtLogInfo("initializing scene");
      pxScene2dRef scene = new pxScene2d;
      scene->init();
      this_->mJavaScene.Reset(isolate, rtObjectWrapper::createFromObjectReference(isolate, scene.getPtr()));

      this_->mView = scene;
      this_->mView->setViewContainer(this_);
      this_->mView->onSize(mWidth, mHeight);
    }

    return PersistentToLocal(isolate, mJavaScene);
  }
  
  

  void startEventProcessingThread()
  {
#ifdef RUNINMAIN
    gLoop = mEventLoop;
#else
    EventLoopContext* ctx = new EventLoopContext();
    ctx->eventLoop = mEventLoop;
#ifdef WIN32
    uintptr_t threadId = _beginthread(processEventLoop, 0, ctx);
    if (threadId != -1)
      mEventLoopThread = (HANDLE)threadId;
#else
    pthread_create(&mEventLoopThread, NULL, &processEventLoop, ctx);
#endif
#endif

  }

#endif // USE_UV_DUMMY_EVENTS

  virtual ~xxWindow()
  { 
    // empty
  }

  pxError setView(pxIView* v)
  {
    mView = v;

    if (v)
    {
      v->setViewContainer(this);
      onSize(mWidth,mHeight);
    }
      
    return PX_OK;
  }


#if 1
  virtual void RT_STDCALL invalidateRect(pxRect* r)
  {
    pxWindow::invalidateRect(r);
  }
#endif


protected:
  static void timerCallback(uv_timer_t* )
  {
    #ifdef RUNINMAIN
    if (gLoop)
      gLoop->runOnce();
    #else
    rtLogDebug("Hello, from uv timer callback");
    #endif
  }

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

  virtual void onCloseRequest()
  {
#ifdef USE_UV_DUMMY_EVENTS    
    uv_timer_stop(&mTimer);
    // mScene->onCloseRequest();
#endif    
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
  }
public:
  rtRefT<pxIView> mView;
private:
  // TODO consolidate with pxCore smart pointer

  Persistent<Object> mJavaScene;
  
  int mWidth;
  int mHeight;

#ifdef USE_UV_DUMMY_EVENTS

  rtRefT<pxIView> mView;
  pxEventLoop* mEventLoop;
 
#ifndef RUNINMAIN
#ifdef WIN32
  HANDLE mEventLoopThread;
#else
  pthread_t mEventLoopThread;
#endif
#endif

 uv_timer_t mTimer;
#endif  //USE_UV_DUMMY_EVENTS

};//CLASS

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // XXWINDOW_H