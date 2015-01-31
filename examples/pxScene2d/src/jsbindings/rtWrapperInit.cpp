#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#include "jsCallback.h"

#include <pxEventLoop.h>
#include <pxScene2d.h>
#include <pxWindow.h>

#include <node.h>

using namespace v8;

struct EventLoopContext
{
  pxEventLoop* eventLoop;
};

static void* ProcessEventLoop(void* argp)
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
    : mCallbacks(new Persistent<Function>[12])
    , mEventLoop(new pxEventLoop())
    , mScene(new pxScene2d())
  {
    mJavaScene = Persistent<Object>::New(rtObjectWrapper::createFromObjectReference(mScene.getPtr()));

    init(x, y, w, h);
    mScene->init();

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
    pthread_create(&mEventLoopThread, NULL, &ProcessEventLoop, ctx);
  }

  virtual ~jsWindow()
  { 
    delete [] mCallbacks;
  }

  void SetCallback(WindowCallback index, Persistent<Function> callback)
  {
    mCallbacks[index] = callback;
  }

private:
  struct FunctionLookup : public jsIFunctionLookup
  {
    FunctionLookup(jsWindow* parent, WindowCallback index)
      : mParent(parent)
      , mIndex(index) { }

    virtual Persistent<Function> lookup()
    {
      return mParent->getCallback(mIndex);
    }

    virtual Handle<Object> self()
    {
      return mParent->scene();
    }
  private:
    jsWindow* mParent;
    WindowCallback mIndex;
  };

  Persistent<Function> getCallback(WindowCallback index)
  {
    return mCallbacks[index];
  }

protected:
  static void timerCallback(uv_timer_t* timer, int status)
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

  virtual void onDraw(pxSurfaceNative s)
  {
    rtWrapperSceneUpdateEnter();
    mScene->onDraw();
    rtWrapperSceneUpdateExit();
  }
private:
  Persistent<Function>* mCallbacks;
  Persistent<Object> mJavaScene;

  pthread_t mEventLoopThread;
  pxEventLoop* mEventLoop;
  pxScene2dRef mScene;
  uv_timer_t mTimer;
};

static jsWindow* mainWindow = NULL;

static Handle<Value> getScene(const Arguments& args)
{
  if (mainWindow == NULL)
  {
    mainWindow = new jsWindow(0, 0, 1280, 720);

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
}

NODE_MODULE(px, ModuleInit)

