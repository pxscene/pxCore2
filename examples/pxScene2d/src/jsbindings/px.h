#ifndef __PX_JS_BINDINGS_H__
#define __PX_JS_BINDINGS_H__

#include <node.h>
#include <v8.h>
#include <vector>

#include <rtValue.h>
#include <pxScene2d.h>
#include <rtObject.h>

class pxEventLoop;
class pxObject;
class pxOffscreen;
class pxScene2d;
class pxWindow;


#define PX_DECL_PROPGET(NAME) static v8::Handle<v8::Value> Get ## NAME(v8::Local<v8::String> prop, const v8::AccessorInfo& info)
#define PX_DECL_PROPSET(NAME) static void Set ## NAME(v8::Local<v8::String> prop, v8::Local<v8::Value> value, const v8::AccessorInfo& info)

#define PX_DECL_FUNC(NAME) static v8::Handle<v8::Value> NAME(const v8::Arguments& args);

#define PXPTR(P) (P)->m_obj


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
  } while (0);

template<typename WrapperType, typename PXObjectType> 
class WrapperObject : public node::ObjectWrap
{
protected:
  WrapperObject(PXObjectType* obj) : m_obj(obj) { m_obj->AddRef(); }
  virtual ~WrapperObject()  { m_obj->Release(); }

  static PXObjectType* unwrap(const v8::AccessorInfo& info)
    { return node::ObjectWrap::Unwrap<WrapperType>(info.This())->m_obj; }
  static PXObjectType* unwrap(const v8::Arguments& args)
    { return node::ObjectWrap::Unwrap<WrapperType>(args.This())->m_obj; }
protected:
  PXObjectType* m_obj;
};


namespace rt
{
  v8::Handle<v8::Value> rt2js(const rtValue& val);
  rtValue js2rt(const v8::Handle<v8::Value>& val);

  class Object : public WrapperObject<Object, rtObject>
  {
  public:
    Object(rtObject* obj) : WrapperObject<Object, rtObject>(obj) { }
    virtual ~Object() { }
  public:
    static void Inherit(v8::Local<v8::FunctionTemplate> derived);
  private:
    PX_DECL_FUNC(Set);
    PX_DECL_FUNC(Get);
    PX_DECL_PROPSET(Property);
    PX_DECL_PROPGET(Property);
  };

  class Function : public WrapperObject<Function, rtIFunction>
  {
  public:
    static v8::Handle<v8::Object> New(const rtFunctionRef& func);
    static void Export(v8::Handle<v8::Object> exports);
  private:
    Function(rtIFunction* obj) : WrapperObject<Function, rtIFunction>(obj) { }
    virtual ~Function() { }

    PX_DECL_FUNC(New);
    PX_DECL_FUNC(Invoke);
  private:
    static v8::Persistent<v8::Function> m_ctor;
  };
}

namespace px
{
  struct IPersistentFunctionLookup
  {
    virtual ~IPersistentFunctionLookup() { }
    virtual v8::Persistent<v8::Function> Lookup() = 0;
  };

  struct JavaScriptCallback
  {
    virtual v8::Handle<v8::Value>* MakeArgs();
    virtual void Enqueue();

    static JavaScriptCallback* New() { return new JavaScriptCallback(); }

    JavaScriptCallback* AddArg(const rtValue& val)
      { m_args.push_back(val); return this; }

    JavaScriptCallback* SetFunctionLookup(IPersistentFunctionLookup* functionLookup)
      { m_functionLookup = functionLookup; return this; }

    static void Work(uv_work_t* req);
    static void DoCallback(uv_work_t* req, int status);

  private:
    std::vector<rtValue> m_args;
    uv_work_t m_req;
    IPersistentFunctionLookup* m_functionLookup;

    JavaScriptCallback(); 
    virtual ~JavaScriptCallback();
  };

  class Window : public node::ObjectWrap
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
    PX_DECL_PROPGET(Scene);
  private:
    static v8::Persistent<v8::Function> m_ctor;
    static pxWindow* unwrap(const v8::AccessorInfo& info)
    { return node::ObjectWrap::Unwrap<Window>(info.This())->m_obj; }
    static pxWindow* unwrap(const v8::Arguments& args)
    { return node::ObjectWrap::Unwrap<Window>(args.This())->m_obj; }
  protected:
    pxWindow* m_obj;
  };

  class EventLoop : public node::ObjectWrap
  {
  public:
    static void Export(v8::Handle<v8::Object> exports);
  private:
    PX_DECL_FUNC(New);
    PX_DECL_FUNC(Run);
    PX_DECL_FUNC(Exit);
  private:
    static v8::Persistent<v8::Function> m_ctor;
    static pxEventLoop* unwrap(const v8::AccessorInfo& info)
      { return node::ObjectWrap::Unwrap<EventLoop>(info.This())->m_obj; }
    static pxEventLoop* unwrap(const v8::Arguments& args)
      { return node::ObjectWrap::Unwrap<EventLoop>(args.This())->m_obj; }
  private:
    pxEventLoop* m_obj;
  };

  class Offscreen : public node::ObjectWrap
  {
  public:
    static void Export(v8::Handle<v8::Object> exports);
  private:
    PX_DECL_FUNC(New);
    PX_DECL_FUNC(Init);
    PX_DECL_FUNC(InitWithColor);
  private:
    static pxOffscreen* unwrap(const v8::AccessorInfo& info)
      { return node::ObjectWrap::Unwrap<Offscreen>(info.This())->m_obj; }
    static pxOffscreen* unwrap(const v8::Arguments& args)
      { return node::ObjectWrap::Unwrap<Offscreen>(args.This())->m_obj; }
    static v8::Persistent<v8::Function> m_ctor;
  private:
    pxOffscreen* m_obj;
  };


  namespace scene
  {
    class Scene2d : public rt::Object
    {
      public:
        static v8::Handle<v8::Object> New(const pxScene2dRef& scene);
        static void Export(v8::Handle<v8::Object> exports);
      private:
        Scene2d(pxScene2d* s) : rt::Object(s) { }
        virtual ~Scene2d() { }
        PX_DECL_FUNC(New);
        PX_DECL_FUNC(GetRoot);
      private:
        static v8::Persistent<v8::Function> m_ctor;
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

