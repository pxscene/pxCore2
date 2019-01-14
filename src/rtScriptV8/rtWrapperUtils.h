/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef RT_WRAPPER_UTILS
#define RT_WRAPPER_UTILS

#include "headers.h"

#include <rtError.h>
#include <rtObject.h>
#include <rtString.h>
#include <rtValue.h>

#include "rtScript.h"

#include <stdarg.h>
#include <string>
#include <map>
#include <memory>

#include <assert.h>

bool rtIsMainThreadNode();

namespace rtScriptV8NodeUtils
{

typedef void(*FuncCallback)(const v8::FunctionCallbackInfo<v8::Value>& args);

struct rtV8FunctionItem
{
  const char  *mName;
  FuncCallback mCallback;
};

//bool rtIsRenderThread();
bool rtIsPromise(const rtValue& v);

template<class T>
inline std::string jsToString(T const& val)
{
  v8::String::Utf8Value v(val->ToString());
  return std::string(*v);
}

template <class TypeName>
inline v8::Local<TypeName> _StrongPersistentToLocal(const v8::Persistent<TypeName>& persistent) 
{
  return *reinterpret_cast<v8::Local<TypeName>*>(
      const_cast<v8::Persistent<TypeName>*>(&persistent));
}

template <class TypeName>
inline v8::Local<TypeName> _WeakPersistentToLocal(v8::Isolate* isolate, const v8::Persistent<TypeName>& persistent) 
{
  return v8::Local<TypeName>::New(isolate, persistent);
}

template <class TypeName>
inline v8::Local<TypeName> PersistentToLocal(v8::Isolate* isolate, const v8::Persistent<TypeName>& persistent) 
{
  if (persistent.IsWeak()) 
    return _WeakPersistentToLocal(isolate, persistent);
  else if (persistent.IsNearDeath())
    return v8::Local<TypeName>();
  else 
    return _StrongPersistentToLocal(persistent);
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
    i = (int)args[which]->IntegerValue();
  return i;
}

class V8ObjectWrap {
public:
  V8ObjectWrap() {
    refs_ = 0;
  }


  virtual ~V8ObjectWrap() {
    if (persistent().IsEmpty())
      return;
    assert(persistent().IsNearDeath());
    persistent().ClearWeak();
    persistent().Reset();
  }


  template <class T>
  static inline T* Unwrap(v8::Local<v8::Object> handle) {
    assert(!handle.IsEmpty());
    assert(handle->InternalFieldCount() > 0);
    // Cast to V8ObjectWrap before casting to T.  A direct cast from void
    // to T won't work right when T has more than one base class.
    void* ptr = handle->GetAlignedPointerFromInternalField(0);
    V8ObjectWrap* wrap = static_cast<V8ObjectWrap*>(ptr);
    return static_cast<T*>(wrap);
  }


  inline v8::Local<v8::Object> handle() {
    return handle(v8::Isolate::GetCurrent());
  }


  inline v8::Local<v8::Object> handle(v8::Isolate* isolate) {
    return v8::Local<v8::Object>::New(isolate, persistent());
  }


  inline v8::Persistent<v8::Object>& persistent() {
    return handle_;
  }


protected:
  inline void Wrap(v8::Local<v8::Object> handle) {
    assert(persistent().IsEmpty());
    assert(handle->InternalFieldCount() > 0);
    handle->SetAlignedPointerInInternalField(0, this);
    persistent().Reset(v8::Isolate::GetCurrent(), handle);
    MakeWeak();
  }


  inline void MakeWeak(void) {
    persistent().SetWeak(this, WeakCallback, v8::WeakCallbackType::kParameter);
    persistent().MarkIndependent();
  }

  /* Ref() marks the object as being attached to an event loop.
  * Refed objects will not be garbage collected, even if
  * all references are lost.
  */
  virtual void Ref() {
    assert(!persistent().IsEmpty());
    persistent().ClearWeak();
    refs_++;
  }

  /* Unref() marks an object as detached from the event loop.  This is its
  * default state.  When an object with a "weak" reference changes from
  * attached to detached state it will be freed. Be careful not to access
  * the object after making this call as it might be gone!
  * (A "weak reference" means an object that only has a
  * persistent handle.)
  *
  * DO NOT CALL THIS FROM DESTRUCTOR
  */
  virtual void Unref() {
    assert(!persistent().IsEmpty());
    assert(!persistent().IsWeak());
    assert(refs_ > 0);
    if (--refs_ == 0)
      MakeWeak();
  }

  int refs_;  // ro

private:
  static void WeakCallback(
    const v8::WeakCallbackInfo<V8ObjectWrap>& data) {
    V8ObjectWrap* wrap = data.GetParameter();
    assert(wrap->refs_ == 0);
    wrap->handle_.Reset();
    delete wrap;
  }

  v8::Persistent<v8::Object> handle_;
};

#define OBJECT_WRAP_CLASS V8ObjectWrap

template<typename TRef, typename TWrapper>
class rtWrapper : public OBJECT_WRAP_CLASS
{
protected:
  rtWrapper(const TRef& ref) : mWrappedObject(ref)
  {
  }

  virtual ~rtWrapper(){ }

  static TRef unwrap(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    return OBJECT_WRAP_CLASS::Unwrap<TWrapper>(args.This())->mWrappedObject;
  }

  static TRef unwrap(const v8::PropertyCallbackInfo<v8::Value>& args)
  {
    return OBJECT_WRAP_CLASS::Unwrap<TWrapper>(args.This())->mWrappedObject;
  }

  static TRef unwrap(const v8::Local<v8::Object>& obj)
  {
    return OBJECT_WRAP_CLASS::Unwrap<TWrapper>(obj)->mWrappedObject;
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
      strncat(buff, rtStrError(err), kBuffSize - strlen(buff) - 1);
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
  static void printAll();
};




} // namespace

#endif

