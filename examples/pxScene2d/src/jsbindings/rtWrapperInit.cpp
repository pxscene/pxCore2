#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#include "jsCallback.h"

#include <pxEventLoop.h>
#include <pxScene2d.h>
#include <pxWindow.h>
#include <pxWindowUtil.h>

using namespace v8;

struct EventLoopContext
{
  pxEventLoop* eventLoop;
};

static void* processEventLoop(void* argp)
{
  EventLoopContext* ctx = reinterpret_cast<EventLoopContext *>(argp);
  ctx->eventLoop->run();
  return 0;
}

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
  eDraw = 11
};


class jsWindow : public pxWindow
{
public:
  jsWindow(int x, int y, int w, int h)
    : mEventLoop(new pxEventLoop())
    , mScene(new pxScene2d())
  {
    mJavaScene = Persistent<Object>::New(rtObjectWrapper::createFromObjectReference(mScene.getPtr()));

    rtLogInfo("creating native with [%d, %d, %d, %d]", x, y, w, h);
    init(x, y, w, h);

    rtLogInfo("initializing scene");
    mScene->init();

    rtLogInfo("starting background thread for event loop processing");
    startEventProcessingThread();

    // we start a timer in case there aren't any other evens to the keep the
    // nodejs event loop alive. Fire a time repeatedly.
    uv_timer_init(uv_default_loop(), &mTimer);
    uv_timer_start(&mTimer, timerCallback, 1000, 1000);
  }

  const Persistent<Object> scene() const
  {
    return mJavaScene;
  }

  void startEventProcessingThread()
  {
    EventLoopContext* ctx = new EventLoopContext();
    ctx->eventLoop = mEventLoop;
    pthread_create(&mEventLoopThread, NULL, &processEventLoop, ctx);
  }

  virtual ~jsWindow()
  { 
    // empty
  }

protected:
  static void timerCallback(uv_timer_t* , int )
  {
    rtLogDebug("Hello, from uv timer callback");
  }

  virtual void onSize(int32_t w, int32_t h)
  {
    mScene->onSize(w, h);
  }

  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    mScene->onMouseDown(x, y, flags);
  }

  virtual void onCloseRequest()
  {
    uv_timer_stop(&mTimer);
    // mScene->onCloseRequest();
  }

  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags)
  {
    mScene->onMouseUp(x, y, flags);
  }

  virtual void onMouseLeave()
  {
    mScene->onMouseLeave();
  }

  virtual void onMouseMove(int32_t x, int32_t y)
  {
    mScene->onMouseMove(x, y);
  }

  virtual void onKeyDown(uint32_t keycode, uint32_t flags)
  {
    mScene->onKeyDown(keycode, flags);
  }

  virtual void onKeyUp(uint32_t keycode, uint32_t flags)
  {
    mScene->onKeyUp(keycode, flags);
  }
  
  virtual void onChar(uint32_t c)
  {
    mScene->onChar(c);
  }

  virtual void onDraw(pxSurfaceNative )
  {
    rtWrapperSceneUpdateEnter();
    mScene->onDraw();
    rtWrapperSceneUpdateExit();
  }
private:
  Persistent<Object> mJavaScene;

  pthread_t mEventLoopThread;
  pxEventLoop* mEventLoop;
  pxScene2dRef mScene;
  uv_timer_t mTimer;
};

static jsWindow* mainWindow = NULL;

static Handle<Value> disposeNode(const Arguments& args)
{
  if (args.Length() < 1)
    return Undefined();

  if (!args[0]->IsObject())
    return Undefined();

  Local<Object> obj = args[0]->ToObject();

  rtObjectWrapper* wrapper = static_cast<rtObjectWrapper *>(obj->GetPointerFromInternalField(0));
  if (wrapper)
    wrapper->dispose();

  return Undefined();
}

static Handle<Value> getScene(const Arguments& args)
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
      x = toInt32(args, 0, 0);
      y = toInt32(args, 1, 0);
      w = toInt32(args, 2, 960);
      h = toInt32(args, 3, 640);
    }

    mainWindow = new jsWindow(x, y, w, h);

    char title[]= { "pxScene from JavasScript!" };
    mainWindow->setTitle(title);
    mainWindow->setVisibility(true);
  }

  return mainWindow->scene();
}

void ModuleInit(Handle<Object> exports) 
{
  rtFunctionWrapper::exportPrototype(exports);
  rtObjectWrapper::exportPrototype(exports);
  exports->Set(String::NewSymbol("getScene"), FunctionTemplate::New(getScene)->GetFunction());
  exports->Set(String::NewSymbol("dispose"), FunctionTemplate::New(disposeNode)->GetFunction());
}

NODE_MODULE(px, ModuleInit)

