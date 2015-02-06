#ifndef RT_OBJECT_WRAPPER_H
#define RT_OBJECT_WRAPPER_H

#include "rtWrapperUtils.h"

using namespace v8;

class rtObjectWrapper : public rtWrapper<rtObjectRef, rtObjectWrapper>
{
public:
  rtObjectWrapper(const rtObjectRef& obj);
  virtual ~rtObjectWrapper(); 

public:
  static void exportPrototype(Handle<Object> exports);

  static Handle<Object> createFromObjectReference(const rtObjectRef& ref);

  static rtValue unwrapObject(const Local<Object>& val);

private:
  static Handle<Value> create(const Arguments& args);

  static Handle<Value> getPropertyByName(Local<String> prop, const AccessorInfo& info);
  static Handle<Value> setPropertyByName(Local<String> prop, Local<Value> val, const AccessorInfo& info);
  static Handle<Array> getEnumerablePropertyNames(const AccessorInfo& info);

  static Handle<Value> getPropertyByIndex(uint32_t index, const AccessorInfo& info);
  static Handle<Value> setPropertyByIndex(uint32_t index, Local<Value> val, const AccessorInfo& info);
  static Handle<Array> getEnumerablePropertyIndecies(const AccessorInfo& info);

  template<typename T>
  static Handle<Value> getProperty(const T& prop, const AccessorInfo& info);

  template<typename T>
  static Handle<Value> setProperty(const T& prop, Local<Value> val, const AccessorInfo& info);

  typedef Handle<Value> (*enumerable_item_creator_t)(rtObjectRef& keys, uint32_t index);
  static Handle<Array> getEnumerable(const AccessorInfo& info, enumerable_item_creator_t create);
};

class jsObjectWrapper : public rtIObject
{
public:
  jsObjectWrapper(const Handle<Value>& val);
  ~jsObjectWrapper();

  virtual unsigned long AddRef();
  virtual unsigned long Release();

  virtual rtError Get(const char* name, rtValue* value);
  virtual rtError Get(uint32_t i, rtValue* value);
  virtual rtError Set(const char* name, const rtValue* value);
  virtual rtError Set(uint32_t i, const rtValue* value);

private:
  rtError getAllKeys(rtValue* value);

private:
  unsigned long mRefCount;
  Persistent<Object> mObject;
};



#endif
#ifndef RT_WRAPPER_UTILS
#define RT_WRAPPER_UTILS

#include <node.h>
#include <v8.h>

#include <rtObject.h>
#include <rtString.h>
#include <rtValue.h>

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

inline int toInt32(const v8::Arguments& args, int which, int defaultValue = 0)
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
  rtWrapper(const TRef& ref) : mWrappedObject(ref) { }
  virtual ~rtWrapper(){ }

  static TRef unwrap(const Local<Object>& obj)
  {
    return node::ObjectWrap::Unwrap<TWrapper>(args.This())->mWrappedObject;
  }

  static TRef unwrap(const v8::Arguments& args)
  {
    return node::ObjectWrap::Unwrap<TWrapper>(args.This())->mWrappedObject;
  }

  static TRef unwrap(const v8::AccessorInfo& info)
  {
    return node::ObjectWrap::Unwrap<TWrapper>(info.This())->mWrappedObject;
  }

  TRef mWrappedObject;
};

rtValue js2rt(const v8::Handle<v8::Value>& val);

v8::Handle<v8::Value> rt2js(const rtValue& val);



#endif

