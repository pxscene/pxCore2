#include "px.h"
#include <pxEventLoop.h>
#include <pxWindow.h>
#include <pxScene2d.h>

#include <string>
#include <pthread.h>

using namespace v8;

namespace
{
  const char* kClassName = "Window";

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

  void* ProcessEventLoop(void* argp)
  {
    pxEventLoop* eventLoop = reinterpret_cast<pxEventLoop *>(argp);
    eventLoop->run();
    return 0;
  }

  class jsWindow : public pxWindow
  {
  public:
    jsWindow(int x, int y, int w, int h)
      : m_callbacks(new Persistent<Function>[12])
      , m_eventLoop(new pxEventLoop())
      , m_scene(new pxScene2d())
    {
      m_javaScene = Persistent<Object>::New(px::scene::Scene2d::New(m_scene));

      init(x, y, w, h);
      startEventProcessingThread();
    }
    
    const Persistent<Object> scene() const
    {
        return m_javaScene;
    }

    void startEventProcessingThread()
    {
      pthread_create(&m_eventLoopThread, NULL, &ProcessEventLoop, m_eventLoop);
    }

    virtual ~jsWindow()
    { 
      delete [] m_callbacks;
    }

    void SetCallback(WindowCallback index, Persistent<Function> callback)
    {
      m_callbacks[index] = callback;
    }

  private:
    struct FunctionLookup : public px::IPersistentFunctionLookup
    {
      FunctionLookup(jsWindow* parent, WindowCallback index)
        : m_parent(parent)
        , m_index(index) { }

      virtual Persistent<Function> Lookup()
      {
        return m_parent->GetCallback(m_index);
      }
    private:
      jsWindow* m_parent;
      WindowCallback m_index;
    };

    Persistent<Function> GetCallback(WindowCallback index)
    {
      Persistent<Function> func;
      func = m_callbacks[index];
      return func;
    }

  protected:
    virtual void onSize(int w, int h)
    {
      px::JavaScriptCallback::New()
        ->AddArg(w)
        ->AddArg(h)
        ->SetFunctionLookup(new FunctionLookup(this, eResize))
        ->Enqueue();
    }

    virtual void onMouseDown(int x, int y, unsigned long flags)
    {
      px::JavaScriptCallback::New()
        ->AddArg(x)
        ->AddArg(y)
        ->AddArg(flags)
        ->SetFunctionLookup(new FunctionLookup(this, eMouseDown))
        ->Enqueue();
    }

    virtual void onCloseRequest()
    {
      px::JavaScriptCallback::New()
        ->SetFunctionLookup(new FunctionLookup(this, eCloseRequest))
        ->Enqueue();
    }

    virtual void onMouseUp(int x, int y, unsigned long flags)
    {
      px::JavaScriptCallback::New()
        ->AddArg(x)
        ->AddArg(y)
        ->AddArg(flags)
        ->SetFunctionLookup(new FunctionLookup(this, eMouseUp))
        ->Enqueue();
    }

    virtual void onMouseLeave()
    {
      px::JavaScriptCallback::New()
        ->SetFunctionLookup(new FunctionLookup(this, eMouseLeave))
        ->Enqueue();
    }

    virtual void onMouseMove(int x, int y)
    {
      px::JavaScriptCallback::New()
        ->AddArg(x)
        ->AddArg(y)
        ->SetFunctionLookup(new FunctionLookup(this, eMouseMove))
        ->Enqueue();
    }

    virtual void onKeyDown(int keycode, unsigned long flags)
    {
      px::JavaScriptCallback::New()
        ->AddArg(keycode)
        ->AddArg(flags)
        ->SetFunctionLookup(new FunctionLookup(this, eKeyDown))
        ->Enqueue();
    }

    virtual void onKeyUp(int keycode, unsigned long flags)
    {
      px::JavaScriptCallback::New()
        ->AddArg(keycode)
        ->AddArg(flags)
        ->SetFunctionLookup(new FunctionLookup(this, eKeyUp))
        ->Enqueue();
    }

    virtual void onDraw(pxSurfaceNative s)
    {
      m_scene->onDraw();
    }
  private:
    Persistent<Function>* m_callbacks;
    Persistent<Object> m_javaScene;

    pthread_t m_eventLoopThread;
    pxEventLoop* m_eventLoop;
    pxScene2dRef m_scene;
  };
}

namespace px
{
  Persistent<Function> Window::m_ctor;

  void Window::Export(Handle<Object> exports)
  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(New);
    tmpl->SetClassName(String::NewSymbol(kClassName));

    Local<Template> proto = tmpl->PrototypeTemplate();
    proto->Set(String::NewSymbol("close"), FunctionTemplate::New(Close)->GetFunction());
    proto->Set(String::NewSymbol("on"), FunctionTemplate::New(OnEvent)->GetFunction());

    Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
    inst->SetInternalFieldCount(1);
    inst->SetAccessor(String::New("visible"), Window::GetVisible, Window::SetVisible);
    inst->SetAccessor(String::New("title"), NULL, Window::SetTitle);
    inst->SetAccessor(String::New("scene"), Window::GetScene, NULL);

    m_ctor = Persistent<Function>::New(tmpl->GetFunction());
    exports->Set(String::NewSymbol(kClassName), m_ctor);
  }

  void Window::SetTitle(Local<String> prop, Local<Value> value, const AccessorInfo& info)
  {
    String::Utf8Value s(value->ToString());
    unwrap(info)->setTitle(*s);
  }

  v8::Handle<v8::Value> Window::GetVisible(Local<String> prop, const AccessorInfo& info)
  {
    return Boolean::New(unwrap(info)->visibility());
  }

  v8::Handle<v8::Value> Window::GetScene(Local<String> prop, const AccessorInfo& info)
  {
    jsWindow* win = static_cast<jsWindow *>(node::ObjectWrap::Unwrap<px::Window>(info.This())->m_obj);
    return win->scene();
  }

  void Window::SetVisible(Local<String> prop, Local<Value> value, const AccessorInfo& info)
  {
    unwrap(info)->setVisibility(value->BooleanValue());
  }

  Handle<Value> Window::OnEvent(const Arguments& args)
  {
    HandleScope scope;

    CHECK_ARGLENGTH(2);

    if (!args[1]->IsFunction())
      PX_THROW(TypeError, "second argument is not callable");

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
      PX_THROW(RangeError, "invalid event: %s", name.c_str());
    }

    static_cast<jsWindow *>(unwrap(args))
      ->SetCallback(index, Persistent<Function>::New(Local<Function>::Cast(args[1])));

    return scope.Close(Undefined());
  }

  Handle<Value> Window::Close(const Arguments& args)
  {
    HandleScope scope;
    unwrap(args)->close();
    return scope.Close(Undefined());
  }

  Handle<Value> Window::New(const Arguments& args)
  { 
    if (args.IsConstructCall())
    {
      Window* w = new Window();
      PXPTR(w) = new jsWindow(toInt32(args, 0), toInt32(args, 1), toInt32(args, 2), toInt32(args, 3));
      w->Wrap(args.This());
      return args.This();
    }
    else
    {
      const int argc = 1;

      HandleScope scope;
      Local<Value> argv[argc] = { args[0] };
      return scope.Close(m_ctor->NewInstance(argc, argv));
    }
  }
}

