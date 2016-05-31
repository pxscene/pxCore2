#ifndef RT_WRAPPER_UTILS
#define RT_WRAPPER_UTILS

#include "node_headers.h"

#include <rtError.h>
#include <rtObject.h>
#include <rtString.h>
#include <rtValue.h>

#include <stdarg.h>
#include <string>
#include <map>
#include <memory>

bool rtIsMainThread();
bool rtIsRenderThread();
bool rtIsPromise(const rtValue& v);

template<class T>
inline std::string jsToString(T const& val)
{
  v8::String::Utf8Value v(val->ToString());
  return std::string(*v);
}

template <class TypeName>
inline v8::Local<TypeName> StrongPersistentToLocal(const v8::Persistent<TypeName>& persistent) 
{
  return *reinterpret_cast<v8::Local<TypeName>*>(
      const_cast<v8::Persistent<TypeName>*>(&persistent));
}

template <class TypeName>
inline v8::Local<TypeName> WeakPersistentToLocal(v8::Isolate* isolate, const v8::Persistent<TypeName>& persistent) 
{
  return v8::Local<TypeName>::New(isolate, persistent);
}

template <class TypeName>
inline v8::Local<TypeName> PersistentToLocal(v8::Isolate* isolate, const v8::Persistent<TypeName>& persistent) 
{
  if (persistent.IsWeak()) 
    return WeakPersistentToLocal(isolate, persistent);
  else 
    return StrongPersistentToLocal(persistent);
}

uint32_t GetContextId(v8::Local<v8::Context>& ctx);

// I don't think node/v8 addons support c++ exceptions. In cases where
// a pending error might be set on a call stack, you can use this object
// to pass around. If there's a pending error, simply use:
// rtWrapperError error;
// someFunction(arg1, arg2, &error);
// if (error.hasError())
//    v8::ThrowException(error.toTypeError());
// use the correct flavor of javascript exception.
class rtWrapperError
{
public:
  rtWrapperError() { }
  rtWrapperError(const char* errorMessage)
    : mMessage(errorMessage) { }

  inline bool hasError() const
    { return !mMessage.empty(); }

  v8::Local<v8::Value> toTypeError(v8::Isolate* isolate)
  {
    return v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, mMessage.c_str()));
  }

  v8::Local<v8::Value> toRangeError(v8::Isolate* isolate)
  {
    return v8::Exception::RangeError(v8::String::NewFromUtf8(isolate, mMessage.c_str()));
  }

  v8::Local<v8::Value> toReferenceError(v8::Isolate* isolate)
  {
    return v8::Exception::ReferenceError(v8::String::NewFromUtf8(isolate, mMessage.c_str()));
  }

  v8::Local<v8::Value> toSyntaxError(v8::Isolate* isolate)
  {
    return v8::Exception::SyntaxError(v8::String::NewFromUtf8(isolate, mMessage.c_str()));
  }

  v8::Local<v8::Value> toGenericError(v8::Isolate* isolate)
  {
    return v8::Exception::Error(v8::String::NewFromUtf8(isolate, mMessage.c_str()));
  }

  void setMessage(const char* errorMessage)
    { mMessage = errorMessage; }

private:
  std::string mMessage;
};

inline rtString toString(const v8::Handle<v8::Object>& obj)
{
  v8::String::Utf8Value utf(obj->ToString());
  return rtString(*utf);
}

inline rtString toString(const v8::Handle<v8::Value>& val)
{
  v8::String::Utf8Value utf(val->ToString());
  return rtString(*utf);
}

inline rtString toString(const v8::Local<v8::String>& s)
{
  v8::String::Utf8Value utf(s);
  return rtString(*utf);
}

inline int toInt32(const v8::FunctionCallbackInfo<v8::Value>& args, int which, int defaultValue = 0)
{
  int i = defaultValue;
  if (!args[which]->IsUndefined())
    i = args[which]->IntegerValue();
  return i;
}

template<typename TRef, typename TWrapper>
class rtWrapper : public node::ObjectWrap
{
protected:
  rtWrapper(const TRef& ref) : mWrappedObject(ref)
  {
  }

  virtual ~rtWrapper(){ }

  static TRef unwrap(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    return node::ObjectWrap::Unwrap<TWrapper>(args.This())->mWrappedObject;
  }

  static TRef unwrap(const v8::PropertyCallbackInfo<v8::Value>& args)
  {
    return node::ObjectWrap::Unwrap<TWrapper>(args.This())->mWrappedObject;
  }

  static TRef unwrap(const v8::Local<v8::Object>& obj)
  {
    return node::ObjectWrap::Unwrap<TWrapper>(obj)->mWrappedObject;
  }

  static void throwRtError(v8::Isolate* isolate, rtError err, const char* format, ...)
    RT_PRINTF_FORMAT(3, 4)
  {
    const int kBuffSize = 256;
    char buff[kBuffSize];

    va_list ptr;
    va_start(ptr, format);
    int n = vsnprintf(buff, sizeof(buff), format, ptr);
    if (n >= kBuffSize)
    {
      buff[kBuffSize - 1] = '\0';
    }
    else
    {
      strcat(buff, ": ");
      strcat(buff, rtStrError(err));
    }
    va_end(ptr);

    isolate->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isolate, buff)));
  }

protected:
  TRef mWrappedObject;
};

rtValue js2rt(v8::Local<v8::Context>& ctx, const v8::Handle<v8::Value>& val, rtWrapperError* error);

v8::Handle<v8::Value> rt2js(v8::Local<v8::Context>& ctx, const rtValue& val);


class HandleMap
{
public:
  static int const kContextIdIndex;

  static void addWeakReference(v8::Isolate* isolate, const rtObjectRef& from, v8::Local<v8::Object>& to);
  static v8::Local<v8::Object> lookupSurrogate(v8::Local<v8::Context>& ctx, const rtObjectRef& from);
  static void clearAllForContext(uint32_t contextId);
};


bool rtWrapperSceneUpdateHasLock();
void rtWrapperSceneUpdateEnter();
void rtWrapperSceneUpdateExit();

class rtWrapperSceneUnlocker
{
public:
  rtWrapperSceneUnlocker()
    : m_hadLock(false)
  {
    if (rtWrapperSceneUpdateHasLock())
    {
      m_hadLock = true;
      rtWrapperSceneUpdateExit();
    }
  }

  ~rtWrapperSceneUnlocker()
  {
    if (m_hadLock)
      rtWrapperSceneUpdateEnter();
  }
private:
  bool m_hadLock;
};

#endif

