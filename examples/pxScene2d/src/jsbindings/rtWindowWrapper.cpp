#include "rtWindowWrapper.h"
#include "rtObjectWrapper.h"
#include "rtWrapperUtils.h"

#include "jsCallback.h"

#include <pxEventLoop.h>
#include <pxWindow.h>
#include <pxScene2d.h>

static const char* kClassName = "Window";
static Persistent<Function> ctor;

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

static void* ProcessEventLoop(void* argp)
{
  pxEventLoop* eventLoop = reinterpret_cast<pxEventLoop *>(argp);
  eventLoop->run();
  return 0;
}




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
  }

  const Persistent<Object> scene() const
  {
    return mJavaScene;
  }

  void startEventProcessingThread()
  {
    pthread_create(&mEventLoopThread, NULL, &ProcessEventLoop, mEventLoop);
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
  private:
    jsWindow* mParent;
    WindowCallback mIndex;
  };

  Persistent<Function> getCallback(WindowCallback index)
  {
    return mCallbacks[index];
  }

protected:
  virtual void onSize(int w, int h)
  {
    jsCallback::create()
      ->addArg(w)
      ->addArg(h)
      ->setFunctionLookup(new FunctionLookup(this, eResize))
      ->enqueue();
  }

  virtual void onMouseDown(int x, int y, unsigned long flags)
  {
    jsCallback::create()
      ->addArg(x)
      ->addArg(y)
      ->addArg(flags)
      ->setFunctionLookup(new FunctionLookup(this, eMouseDown))
      ->enqueue();
  }

  virtual void onCloseRequest()
  {
    jsCallback::create()
      ->setFunctionLookup(new FunctionLookup(this, eCloseRequest))
      ->enqueue();
  }

  virtual void onMouseUp(int x, int y, unsigned long flags)
  {
    jsCallback::create()
      ->addArg(x)
      ->addArg(y)
      ->addArg(flags)
      ->setFunctionLookup(new FunctionLookup(this, eMouseUp))
      ->enqueue();
  }

  virtual void onMouseLeave()
  {
    jsCallback::create()
      ->setFunctionLookup(new FunctionLookup(this, eMouseLeave))
      ->enqueue();
  }

  virtual void onMouseMove(int x, int y)
  {
    jsCallback::create()
      ->addArg(x)
      ->addArg(y)
      ->setFunctionLookup(new FunctionLookup(this, eMouseMove))
      ->enqueue();
  }

  virtual void onKeyDown(int keycode, unsigned long flags)
  {
    jsCallback::create()
      ->addArg(keycode)
      ->addArg(flags)
      ->setFunctionLookup(new FunctionLookup(this, eKeyDown))
      ->enqueue();
  }

  virtual void onKeyUp(int keycode, unsigned long flags)
  {
    jsCallback::create()
      ->addArg(keycode)
      ->addArg(flags)
      ->setFunctionLookup(new FunctionLookup(this, eKeyUp))
      ->enqueue();
  }

  virtual void onDraw(pxSurfaceNative s)
  {
    mScene->onDraw();
  }
private:
  Persistent<Function>* mCallbacks;
  Persistent<Object> mJavaScene;

  pthread_t mEventLoopThread;
  pxEventLoop* mEventLoop;
  pxScene2dRef mScene;
};

void rtWindowWrapper::exportPrototype(Handle<Object> exports)
{
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(create);
  tmpl->SetClassName(String::NewSymbol(kClassName));

  Local<Template> proto = tmpl->PrototypeTemplate();
  proto->Set(String::NewSymbol("close"), FunctionTemplate::New(close)->GetFunction());
  proto->Set(String::NewSymbol("on"), FunctionTemplate::New(on)->GetFunction());

  Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
  inst->SetInternalFieldCount(1);
  inst->SetAccessor(String::New("visible"), rtWindowWrapper::getVisible, rtWindowWrapper::setVisible);
  inst->SetAccessor(String::New("title"), NULL, rtWindowWrapper::setTitle);
  inst->SetAccessor(String::New("scene"), rtWindowWrapper::getScene, NULL);

  ctor = Persistent<Function>::New(tmpl->GetFunction());
  exports->Set(String::NewSymbol(kClassName), ctor);
}

void rtWindowWrapper::setTitle(Local<String> prop, Local<Value> value, const AccessorInfo& info)
{
  String::Utf8Value s(value->ToString());
  unwrap(info)->setTitle(*s);
}

Handle<Value> rtWindowWrapper::getVisible(Local<String> prop, const AccessorInfo& info)
{
  return Boolean::New(unwrap(info)->visibility());
}

Handle<Value> rtWindowWrapper::getScene(Local<String> prop, const AccessorInfo& info)
{
  jsWindow* win = static_cast<jsWindow *>(unwrap(info));
  return win->scene();
}

void rtWindowWrapper::setVisible(Local<String> prop, Local<Value> value, const AccessorInfo& info)
{
  unwrap(info)->setVisibility(value->BooleanValue());
}

Handle<Value> rtWindowWrapper::on(const Arguments& args)
{
  HandleScope scope;

  // CHECK_ARGLENGTH(2);

  if (!args[1]->IsFunction())
  {
    // PX_THROW(TypeError, "second argument is not callable");
    return scope.Close(Undefined());
  }

  String::Utf8Value s(args[0]->ToString());
  std::string name(*s);

  WindowCallback index;

  if (name == "create")             index = eCreate;
  else if (name == "closerequest")  index = eCloseRequest;
  else if (name == "close")         index = eClose;
  else if (name == "resize")        index = eResize;
  else if (name == "mousedown")     index = eMouseDown;
  else if (name == "mouseup")       index = eMouseUp;
  else if (name == "mouseleave")    index = eMouseLeave;
  else if (name == "mousemove")     index = eMouseMove;
  else if (name == "keydown")       index = eKeyDown;
  else if (name == "keyup")         index = eKeyUp;
  else {
    // PX_THROW(RangeError, "invalid event: %s", name.c_str());
    return scope.Close(Undefined());
  }

  static_cast<jsWindow *>(unwrap(args))
    ->SetCallback(index, Persistent<Function>::New(Local<Function>::Cast(args[1])));

  return scope.Close(Undefined());
}

Handle<Value> rtWindowWrapper::close(const Arguments& args)
{
  HandleScope scope;
  unwrap(args)->close();
  return scope.Close(Undefined());
}

Handle<Value> rtWindowWrapper::create(const Arguments& args)
{ 
  if (args.IsConstructCall())
  {
    rtWindowWrapper* wrapper = new rtWindowWrapper();
    wrapper->mWindow = new jsWindow(toInt32(args, 0), toInt32(args, 1), toInt32(args, 2), toInt32(args, 3));
    wrapper->Wrap(args.This());
    return args.This();
  }
  else
  {
    const int argc = 1;

    HandleScope scope;
    Local<Value> argv[argc] = { args[0] };
    return scope.Close(ctor->NewInstance(argc, argv));
  }
}
