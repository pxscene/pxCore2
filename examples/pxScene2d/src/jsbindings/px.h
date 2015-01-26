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

#define PX_DECL_PROPGET(NAME) static v8::Handle<v8::Value> Get ## NAME(v8::Local<v8::String> prop, const v8::AccessorInfo& info)
#define PX_DECL_PROPSET(NAME) static void Set ## NAME(v8::Local<v8::String> prop, v8::Local<v8::Value> value, const v8::AccessorInfo& info)

#define PX_DECL_FUNC(NAME) static v8::Handle<v8::Value> NAME(const v8::Arguments& args);

#define PXPTR(P) (P)->m_pxObject


#define CHECK_ARGLENGTH(N) \
if (args.Length() != (N)) { \
  char buff[256]; \
  snprintf(buff, sizeof(buff), "Expected: %d arguments, but got: %d", args.Length(), (N)); \
  v8::ThrowException(v8::Exception::TypeError(v8::String::New(buff))); \
  return scope.Close(v8::Undefined()); \
}

#define PX_THROW(TYPE, format, ...) \
  do { \
    char buff[256]; \
    snprintf(buff, sizeof(buff), format, ##__VA_ARGS__); \
    v8::ThrowException(v8::Exception::TYPE(v8::String::New(buff))); \
    return scope.Close(v8::Undefined()); \
  } while (0);

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

  struct IPersistentFunctionLookup
  {
    virtual ~IPersistentFunctionLookup() { }
    virtual v8::Persistent<v8::Function> Lookup() = 0;
  };

  struct JavaScriptCallback
  {
    struct Argument
    {
      enum ArgType {
        AT_I2,
        AT_I4,
        AT_UL
      };

      union ArgData {
        int32_t iVal;
        unsigned long ulVal;
        int16_t sVal;
      };

      ArgType Type;
      ArgData Data;

      Argument(int32_t i) : Type(AT_I4) { Data.iVal = i; }
      Argument(int16_t s) : Type(AT_I2) { Data.sVal = s; }
      Argument(unsigned long ul) : Type(AT_UL) { Data.ulVal = ul; }
    };

    virtual v8::Handle<v8::Value>* MakeArgs();
    virtual void Enqueue();

    static JavaScriptCallback* New() { return new JavaScriptCallback(); }

    JavaScriptCallback* AddArg(Argument arg)
      { m_args.push_back(arg); return this; }

    JavaScriptCallback* SetFunctionLookup(IPersistentFunctionLookup* functionLookup)
      { m_functionLookup = functionLookup; return this; }

    static void Work(uv_work_t* req);
    static void DoCallback(uv_work_t* req, int status);

  private:
    std::vector<Argument> m_args;
    uv_work_t m_req;
    IPersistentFunctionLookup* m_functionLookup;

    JavaScriptCallback(); 
    virtual ~JavaScriptCallback();
  };




  class Window : public WrapperObject<Window, pxWindow>
  {
  public:
      static void Export(v8::Handle<v8::Object> exports);
  private:
    PX_DECL_FUNC(New);
    PX_DECL_FUNC(Close);
    PX_DECL_FUNC(OnEvent);

    PX_DECL_PROPGET(Visible);
    PX_DECL_PROPSET(Visible);
    PX_DECL_PROPSET(Title);
  };

  class EventLoop : public WrapperObject<EventLoop, pxEventLoop>
  {
  public:
    static void Export(v8::Handle<v8::Object> exports);
  private:
    PX_DECL_FUNC(New);
    PX_DECL_FUNC(Run);
    PX_DECL_FUNC(Exit);
  };

  class Offscreen : public WrapperObject<Offscreen, pxOffscreen>
  {
  public:
    static void Export(v8::Handle<v8::Object> exports);
  private:
    PX_DECL_FUNC(New);
    PX_DECL_FUNC(Init);
    PX_DECL_FUNC(InitWithColor);
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
        static void Export(v8::Handle<v8::Object> exports);
      private:
        PX_DECL_FUNC(New);
        PX_DECL_FUNC(GetRoot);
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

