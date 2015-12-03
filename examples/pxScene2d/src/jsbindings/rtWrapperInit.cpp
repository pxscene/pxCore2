#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#include "jsCallback.h"

#include <pxEventLoop.h>
#include <pxScene2d.h>
#include <pxWindow.h>
#include <pxWindowUtil.h>
#include <pxTimer.h>

using namespace v8;

#ifdef WIN32
static DWORD __rt_main_thread__;
#else
static pthread_t __rt_main_thread__;
#endif

#ifdef RUNINMAIN
#define ENTERSCENELOCK()
#define EXITSCENELOCK() 
#else
#define ENTERSCENELOCK() rtWrapperSceneUpdateEnter();
#define EXITSCENELOCK() rtWrapperSceneUpdateExit(); 
#endif

bool rtIsMainThread()
{
#ifdef WIN32
  return GetCurrentThreadId() == __rt_main_thread__;
#else
  return pthread_self() == __rt_main_thread__;
#endif
}

struct EventLoopContext
{
  pxEventLoop* eventLoop;
};

// TODO on OSX run the windows event loop on the main thread and use
// a timer to pump messages
#ifndef RUNINMAIN
#ifdef WIN32
static void processEventLoop(void* argp)
#else
static void* processEventLoop(void* argp)
#endif
{
  EventLoopContext* ctx = reinterpret_cast<EventLoopContext *>(argp);
  ctx->eventLoop->run();
#ifndef WIN32
  return 0;
#endif
}
#endif

enum WindowCallback
{
  eCreate = 0,
  eCloseRequest = 1,
  eClose = 2,
  eAnimationTimer = 3,
  eResize = 4,
  eMouseDown = 5,
  eMouseUp = 6,
  eMouseLeave = 7,
  eMouseMove = 8,
  eKeyDown = 9,
  eKeyUp = 10,
  eDraw = 11,
  eFocus = 12,
  eBlur = 13,
};


pxEventLoop* gLoop = NULL;

class jsWindow : public pxWindow, public pxIViewContainer
{
public:
  jsWindow(Isolate* isolate, int x, int y, int w, int h)
    : pxWindow()
    , mScene(new pxScene2d())
    , mEventLoop(new pxEventLoop())
  {
    mJavaScene.Reset(isolate, rtObjectWrapper::createFromObjectReference(isolate, mScene.getPtr()));

    rtLogInfo("creating native with [%d, %d, %d, %d]", x, y, w, h);
    init(x, y, w, h);

    rtLogInfo("initializing scene");
    mScene->init();
    mScene->setViewContainer(this);
  }

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

  virtual ~jsWindow()
  { 
    // empty
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
    ENTERSCENELOCK();
    mScene->onSize(w, h);
    EXITSCENELOCK();
  }

  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    ENTERSCENELOCK();
    mScene->onMouseDown(x, y, flags);
    EXITSCENELOCK();
  }

  virtual void onCloseRequest()
  {
    uv_timer_stop(&mTimer);
    // mScene->onCloseRequest();
  }

  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags)
  {
    ENTERSCENELOCK();
    mScene->onMouseUp(x, y, flags);
    EXITSCENELOCK();
  }

  virtual void onMouseLeave()
  {
    ENTERSCENELOCK();
    mScene->onMouseLeave();
    EXITSCENELOCK();
  }

  virtual void onMouseMove(int32_t x, int32_t y)
  {
    ENTERSCENELOCK();
    mScene->onMouseMove(x, y);
    EXITSCENELOCK();
  }

  virtual void onFocus()
  {
    ENTERSCENELOCK();
    mScene->onFocus();
    EXITSCENELOCK();
  }
  virtual void onBlur()
  {
    ENTERSCENELOCK();
    mScene->onBlur();
    EXITSCENELOCK();
  }

  virtual void onKeyDown(uint32_t keycode, uint32_t flags)
  {
    ENTERSCENELOCK();
    mScene->onKeyDown(keycode, flags);
    EXITSCENELOCK();
  }

  virtual void onKeyUp(uint32_t keycode, uint32_t flags)
  {
    ENTERSCENELOCK();
    mScene->onKeyUp(keycode, flags);
    EXITSCENELOCK();
  }
  
  virtual void onChar(uint32_t c)
  {
    ENTERSCENELOCK();
    mScene->onChar(c);
    EXITSCENELOCK();
  }

  virtual void onDraw(pxSurfaceNative )
  {
    ENTERSCENELOCK();
    mScene->onDraw();
    EXITSCENELOCK();
  }

  virtual void onAnimationTimer()
  {
    ENTERSCENELOCK();
    mScene->onUpdate(pxSeconds());
    EXITSCENELOCK();
  }
private:
  pxScene2dRef mScene;
  pxEventLoop* mEventLoop;
  Persistent<Object> mJavaScene;

#ifndef RUNINMAIN
#ifdef WIN32
  HANDLE mEventLoopThread;
#else
  pthread_t mEventLoopThread;
#endif
#endif

  uv_timer_t mTimer;
};

static jsWindow* mainWindow = NULL;

static void disposeNode(const FunctionCallbackInfo<Value>& args)
{
  if (args.Length() < 1)
    return;

  if (!args[0]->IsObject())
    return;

  Local<Object> obj = args[0]->ToObject();

  rtObjectWrapper* wrapper = static_cast<rtObjectWrapper *>(obj->GetAlignedPointerFromInternalField(0));
  if (wrapper)
    wrapper->dispose();
}

static void getScene(const FunctionCallbackInfo<Value>& args)
{
  if (mainWindow == NULL)
  {
    // This is somewhat experimental. There are concurrency issues with glut.
    // There's no way to intergate glut eventloop with js threads. Once
    // you enter the glut event loop, you don't come out. I'm putting this
    // here to address stability issues with demo apps.
    #if PX_PLATFORM_X11
    XInitThreads();
    #endif

    int x = 0;
    int y = 0;
    int w = 960;
    int h = 640;

    if (args.Length() == 4)
    {
      x = toInt32(args, 0, x);
      y = toInt32(args, 1, y);
      w = toInt32(args, 2, w);
      h = toInt32(args, 3, h);
    }

    mainWindow = new jsWindow(args.GetIsolate(), x, y, w, h);

    char title[]= { "pxScene from JavasScript!" };
    mainWindow->setTitle(title);
    mainWindow->setVisibility(true);
    mainWindow->startTimers();
  }

  EscapableHandleScope scope(args.GetIsolate());
  args.GetReturnValue().Set(scope.Escape(mainWindow->scene(args.GetIsolate())));
}

void ModuleInit(
  Handle<Object>      target,
  Handle<Value>     /* unused */,
  Handle<Context>     context)
{
#ifdef WIN32
  __rt_main_thread__ = GetCurrentThreadId();
#else
  __rt_main_thread__ = pthread_self();
#endif

  Isolate* isolate = context->GetIsolate();

  rtFunctionWrapper::exportPrototype(isolate, target);
  rtObjectWrapper::exportPrototype(isolate, target);

  target->Set(String::NewFromUtf8(isolate, "getScene"),
    FunctionTemplate::New(isolate, getScene)->GetFunction());

  target->Set(String::NewFromUtf8(isolate, "dispose"),
    FunctionTemplate::New(isolate, disposeNode)->GetFunction());
}

NODE_MODULE_CONTEXT_AWARE(px, ModuleInit);
