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

    rtLogInfo("starting background thread for event loop processing");
    startEventProcessingThread();

    // we start a timer in case there aren't any other evens to the keep the
    // nodejs event loop alive. Fire a time repeatedly.
    uv_timer_init(uv_default_loop(), &mTimer);
    uv_timer_start(&mTimer, timerCallback, 1000, 1000);
  }

  Local<Object> scene(Isolate* isolate) const
  {
    return PersistentToLocal(isolate, mJavaScene);
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
  static void timerCallback(uv_timer_t* )
  {
    rtLogDebug("Hello, from uv timer callback");
  }

  virtual void onSize(int w, int h)
  {
    mScene->onSize(w, h);
  }

  virtual void onMouseDown(int x, int y, unsigned long flags)
  {
    mScene->onMouseDown(x, y, flags);
  }

  virtual void onCloseRequest()
  {
    uv_timer_stop(&mTimer);
    // mScene->onCloseRequest();
  }

  virtual void onMouseUp(int x, int y, unsigned long flags)
  {
    mScene->onMouseUp(x, y, flags);
  }

  virtual void onMouseLeave()
  {
    mScene->onMouseLeave();
  }

  virtual void onMouseMove(int x, int y)
  {
    mScene->onMouseMove(x, y);
  }

  virtual void onKeyDown(int keycode, unsigned long flags)
  {
    mScene->onKeyDown(keycode, flags);
  }

  virtual void onKeyUp(int keycode, unsigned long flags)
  {
    mScene->onKeyUp(keycode, flags);
  }
  
  virtual void onChar(char c)
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
  pxScene2dRef mScene;
  pxEventLoop* mEventLoop;
  Persistent<Object> mJavaScene;

  pthread_t mEventLoopThread;
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
      x = toInt32(args, 0, 0);
      y = toInt32(args, 1, 0);
      w = toInt32(args, 2, 960);
      h = toInt32(args, 3, 640);
    }

    mainWindow = new jsWindow(args.GetIsolate(), x, y, w, h);

    char title[]= { "pxScene from JavasScript!" };
    mainWindow->setTitle(title);
    mainWindow->setVisibility(true);
  }

  EscapableHandleScope scope(args.GetIsolate());
  args.GetReturnValue().Set(scope.Escape(mainWindow->scene(args.GetIsolate())));
}

void ModuleInit(
  Handle<Object>      target,
  Handle<Value>     /* unused */,
  Handle<Context>     context)
{
  Isolate* isolate = context->GetIsolate();

  rtFunctionWrapper::exportPrototype(isolate, target);
  rtObjectWrapper::exportPrototype(isolate, target);

  target->Set(String::NewFromUtf8(isolate, "getScene"),
    FunctionTemplate::New(isolate, getScene)->GetFunction());

  target->Set(String::NewFromUtf8(isolate, "dispose"),
    FunctionTemplate::New(isolate, disposeNode)->GetFunction());
}

NODE_MODULE_CONTEXT_AWARE(px, ModuleInit);
