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

  static TRef unwrap(const v8::Arguments& args)
  {
    return node::ObjectWrap::Unwrap<TWrapper>(args.This())->mWrappedObject;
  }

  static TRef unwrap(const v8::Local<v8::Object>& obj)
  {
    return node::ObjectWrap::Unwrap<TWrapper>(obj)->mWrappedObject;
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

