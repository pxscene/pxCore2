#include "px.h"

#define PX_PLATFORM_X11
#include <pxWindow.h>
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

  class jsWindow : public pxWindow
  {
    friend class px::Window;
  public:
    jsWindow()
    {
      m_callbacks = new Persistent<Function>[12];
      pthread_mutex_init(&m_mutex, 0);
    }

    virtual ~jsWindow()
    { 
      pthread_mutex_destroy(&m_mutex);
      delete [] m_callbacks;
    }

    void SetCallback(WindowCallback index, Persistent<Function> callback)
    {
      pthread_mutex_lock(&m_mutex);
      m_callbacks[index] = callback;
      pthread_mutex_unlock(&m_mutex);
    }


  protected:
    virtual void onSize(int w, int h)
    {
      px::AsyncContext* ctx = new px::AsyncContext();
      ctx->Args.push_back(w);
      ctx->Args.push_back(h);
      ctx->Callback = &m_callbacks[eResize];
      ctx->EnqueueCallback();
    }

    virtual void onMouseDown(int x, int y, unsigned long flags)
    {
      px::AsyncContext* ctx = new px::AsyncContext();
      ctx->Args.push_back(x);
      ctx->Args.push_back(y);
      ctx->Args.push_back(flags);
      ctx->Callback = &m_callbacks[eMouseDown];
      ctx->EnqueueCallback();
    }

    virtual void onCloseRequest()
    {
      px::AsyncContext* ctx = new px::AsyncContext();
      ctx->Callback = &m_callbacks[eCloseRequest];
      ctx->EnqueueCallback();
    }

    virtual void onMouseUp(int x, int y, unsigned long flags)
    {
      px::AsyncContext* ctx = new px::AsyncContext();
      ctx->Args.push_back(x);
      ctx->Args.push_back(y);
      ctx->Args.push_back(flags);
      ctx->Callback = &m_callbacks[eMouseUp];
      ctx->EnqueueCallback();
    }

    virtual void onMouseLeave()
    {
      px::AsyncContext* ctx = new px::AsyncContext();
      ctx->Callback = &m_callbacks[eMouseLeave];
      ctx->EnqueueCallback();
    }

    virtual void onMouseMove(int x, int y)
    {
      px::AsyncContext* ctx = new px::AsyncContext();
      ctx->Args.push_back(x);
      ctx->Args.push_back(y);
      ctx->Callback = &m_callbacks[eMouseMove];
      ctx->EnqueueCallback();
    }

    virtual void onKeyDown(int keycode, unsigned long flags)
    {
      px::AsyncContext* ctx = new px::AsyncContext();
      ctx->Args.push_back(keycode);
      ctx->Args.push_back(flags);
      ctx->Callback = &m_callbacks[eKeyDown];
      ctx->EnqueueCallback();
    }

    virtual void onKeyUp(int keycode, unsigned long flags)
    {
      px::AsyncContext* ctx = new px::AsyncContext();
      ctx->Args.push_back(keycode);
      ctx->Args.push_back(flags);
      ctx->Callback = &m_callbacks[eKeyUp];
      ctx->EnqueueCallback();
    }
  private:
    Persistent<Function>* m_callbacks;
    pthread_mutex_t m_mutex;
  };
}

namespace px
{
  template<typename TWrapper, typename TPXObject> 
  Persistent<Function> WrapperObject<TWrapper, TPXObject>::m_ctor;

  void Window::Build(Handle<Object> exports)
  {
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol(kClassName));
    t->InstanceTemplate()->SetInternalFieldCount(1);

    Local<Template> proto = t->PrototypeTemplate();
    proto->Set(String::NewSymbol("close"), FunctionTemplate::New(Close)->GetFunction());
    proto->Set(String::NewSymbol("on"), FunctionTemplate::New(OnEvent)->GetFunction());

    Local<ObjectTemplate> inst = t->InstanceTemplate();
    inst->SetAccessor(String::New("visible"), Window::GetVisible, Window::SetVisible);
    inst->SetAccessor(String::New("title"), NULL, Window::SetTitle);

    m_ctor = Persistent<Function>::New(t->GetFunction());
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

  void Window::SetVisible(Local<String> prop, Local<Value> value, const AccessorInfo& info)
  {
    unwrap(info)->setVisibility(value->BooleanValue());
  }

  Handle<Value> Window::OnEvent(const Arguments& args)
  {
    HandleScope scope;

    CHECK_ARGLENGTH(2);

    if (!args[1]->IsFunction())
    {
      char buff[256];
      snprintf(buff, sizeof(buff), "Second argument is not callbable");
      ThrowException(Exception::TypeError(String::New(buff)));
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
      char buff[256];
      snprintf(buff, sizeof(buff), "Invalid event: %s", name.c_str());
      ThrowException(Exception::TypeError(String::New(buff)));
      return scope.Close(Undefined());
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
      PXPTR(w) = new jsWindow();
      PXPTR(w)->init(toInt32(args, 0), toInt32(args, 1), toInt32(args, 2), toInt32(args, 3));

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

