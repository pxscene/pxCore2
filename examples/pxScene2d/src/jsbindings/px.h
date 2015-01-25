#ifndef __PX_JS_BINDINGS_H__
#define __PX_JS_BINDINGS_H__

#include <node.h>
#include <v8.h>
#include <vector>

class pxEventLoop;
class pxObject;
class pxOffscreen;
class pxScene2d;
class pxWindow;

#define DECL_PROPGET(NAME) static v8::Handle<v8::Value> Get ## NAME(v8::Local<v8::String> prop, const v8::AccessorInfo& info)
#define DECL_PROPSET(NAME) static void Set ## NAME(v8::Local<v8::String> prop, v8::Local<v8::Value> value, const v8::AccessorInfo& info)

#define DECL_FUNC(NAME) static v8::Handle<v8::Value> NAME(const v8::Arguments& args);

#define PXPTR(P) (P)->m_pxObject


#define CHECK_ARGLENGTH(N) \
if (args.Length() != (N)) { \
  char buff[256]; \
  snprintf(buff, sizeof(buff), "Expected: %d arguments, but got: %d", args.Length(), (N)); \
  v8::ThrowException(v8::Exception::TypeError(v8::String::New(buff))); \
  return scope.Close(v8::Undefined()); \
}

namespace px
{
  template<typename TWrapper, typename TPXObject> 
  class WrapperObject : public node::ObjectWrap
  {
  protected:
    static TPXObject* unwrap(const v8::AccessorInfo& info)
    {
      return node::ObjectWrap::Unwrap<TWrapper>(info.This())->m_pxObject;
    }
    static TPXObject* unwrap(const v8::Arguments& args)
    {
      return node::ObjectWrap::Unwrap<TWrapper>(args.This())->m_pxObject;
    }
  protected:
    TPXObject* m_pxObject;
  protected:
    static v8::Persistent<v8::Function> m_ctor;
  };


  struct AsyncContext
  {
    struct Argument
    {
      enum ArgType {
        AT_I4,
        AT_UL
      };

      union ArgData {
        int32_t iVal;
        unsigned long ulVal;
      };

      ArgType Type;
      ArgData Data;

      Argument(int32_t i) : Type(AT_I4) { Data.iVal = i; }
      Argument(unsigned long ul) : Type(AT_UL) { Data.ulVal = ul; }

      v8::Handle<v8::Value> ToJavaScript();
    };

    uv_work_t Request;
    v8::Persistent<v8::Function>* Callback;
    std::vector<Argument> Args;

    AsyncContext();
    virtual ~AsyncContext();

    virtual v8::Handle<v8::Value>* MakeArgs();
    virtual void EnqueueCallback();

    static void Work(uv_work_t* req);
    static void DoCallback(uv_work_t* req, int status);
  };




  class Window : public WrapperObject<Window, pxWindow>
  {
  public:
      static void Build(v8::Handle<v8::Object> exports);
  private:
    DECL_FUNC(New);
    DECL_FUNC(Close);
    DECL_FUNC(OnEvent);

    DECL_PROPGET(Visible);
    DECL_PROPSET(Visible);
    DECL_PROPSET(Title);
  };

  class EventLoop : public WrapperObject<EventLoop, pxEventLoop>
  {
  public:
    static void Build(v8::Handle<v8::Object> exports);
  private:
    DECL_FUNC(New);
    DECL_FUNC(Run);
    DECL_FUNC(Exit);
  };

  class Offscreen : public WrapperObject<Offscreen, pxOffscreen>
  {
  public:
    static void Build(v8::Handle<v8::Object> exports);
  private:
    DECL_FUNC(New);
    DECL_FUNC(Init);
    DECL_FUNC(InitWithColor);
  };




  namespace scene
  {
    class BaseObject
    {
    public:
      static void Inherit(v8::Local<v8::FunctionTemplate> child);
    }; 

    class Scene2d : public node::ObjectWrap
    {
      public:
        static void Build(v8::Handle<v8::Object> exports);
      private:
        DECL_FUNC(New);
        DECL_FUNC(GetRoot);
      private:
        static v8::Persistent<v8::Function> m_ctor;
      private:
        pxScene2d* m_scene;
    };
  }

  inline int toInt32(const v8::Arguments& args, int which, int defaultValue = 0)
  {
    int i = defaultValue;
    if (!args[which]->IsUndefined())
      i = args[which]->IntegerValue();
    return i;
  }

  
}
#endif

