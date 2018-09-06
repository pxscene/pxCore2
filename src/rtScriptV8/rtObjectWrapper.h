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

#ifndef RT_OBJECT_WRAPPER_H
#define RT_OBJECT_WRAPPER_H

#include "rtWrapperUtils.h"

using namespace v8;

namespace rtScriptV8NodeUtils
{

class rtObjectWrapper : public rtWrapper<rtObjectRef, rtObjectWrapper>
{
public:
  rtObjectWrapper(const rtObjectRef& ref);
  virtual ~rtObjectWrapper();

  void dispose()
  {
  }

public:
  static void exportPrototype(Isolate* isolate, Handle<Object> exports);
  static void destroyPrototype();

  static Handle<Object> createFromObjectReference(v8::Local<v8::Context>& ctx, const rtObjectRef& ref);

  static rtValue unwrapObject(const Local<Object>& val);

private:
  static void create(const FunctionCallbackInfo<Value>& args);

  static void getPropertyByName(Local<String> prop, const PropertyCallbackInfo<Value>& info);
  static void setPropertyByName(Local<String> prop, Local<Value> val, const PropertyCallbackInfo<Value>& info);
  static void getEnumerablePropertyNames(const PropertyCallbackInfo<Array>& info);
#ifdef ENABLE_DEBUG_MODE
  static void queryPropertyByName(Local<String> prop, const PropertyCallbackInfo<Integer>& info);
#endif
  static void getPropertyByIndex(uint32_t index, const PropertyCallbackInfo<Value>& info);
  static void setPropertyByIndex(uint32_t index, Local<Value> val, const PropertyCallbackInfo<Value>& info);
  static void getEnumerablePropertyIndecies(const PropertyCallbackInfo<Array>& info);

  template<typename T>
  static void getProperty(const T& prop, const PropertyCallbackInfo<Value>& info);

  template<typename T>
  static void setProperty(const T& prop, Local<Value> val, const PropertyCallbackInfo<Value>& info);

  typedef Handle<Value> (*enumerable_item_creator_t)(v8::Isolate* isolate, rtObjectRef& keys, uint32_t index);
  static void getEnumerable(const PropertyCallbackInfo<Array>& info, enumerable_item_creator_t create);

#ifdef ENABLE_DEBUG_MODE
  template<typename T>
  static void queryProperty(const T& prop, const PropertyCallbackInfo<Integer>& info);
#endif
};

class jsObjectWrapper : public rtIObject
{
public:
  jsObjectWrapper(v8::Isolate* isolate, const Handle<Value>& val, bool isArray);
  virtual ~jsObjectWrapper();

  static const char* kIsJavaScriptObjectWrapper;
  static bool isJavaScriptObjectWrapper(const rtObjectRef& obj);

  virtual unsigned long AddRef();
  virtual unsigned long Release();
  virtual unsigned long getRefCount() const
    { return mRefCount; }

  virtual rtError Get(const char* name, rtValue* value) const;
  virtual rtError Get(uint32_t i, rtValue* value) const;
  virtual rtError Set(const char* name, const rtValue* value);
  virtual rtError Set(uint32_t i, const rtValue* value);
  virtual rtMethodMap* getMap() const { return NULL; }
  Local<Object> getWrappedObject();

private:
  rtError getAllKeys(Isolate* isolate, rtValue* value) const;

private:
  unsigned long mRefCount;
  Persistent<Object> mObject;
  Isolate* mIsolate;
  bool mIsArray;
};

} // namespace

#endif
#ifndef RT_WRAPPER_UTILS
#define RT_WRAPPER_UTILS

#include "headers.h"
#include <rtObject.h>
#include <rtString.h>
#include <rtValue.h>

namespace rtScriptV8NodeUtils
{

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

inline int toInt32(const v8::FunctionCallbackInfo<Value>& args, int which, int defaultValue = 0)
{
  int i = defaultValue;
  if (!args[which]->IsUndefined())
    i = args[which]->IntegerValue();
  return i;
}

rtValue js2rt(const v8::Handle<v8::Value>& val);

v8::Handle<v8::Value> rt2js(const rtValue& val);

} // namespace

#endif

