#ifndef RT_WRAPPER_UTILS
#define RT_WRAPPER_UTILS

#include <node.h>
#include <v8.h>

#include <rtError.h>
#include <rtObject.h>
#include <rtString.h>
#include <rtValue.h>

#include <stdarg.h>
#include <string>

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

  v8::Local<v8::Value> toTypeError()
  {
    return v8::Exception::TypeError(v8::String::New(mMessage.c_str()));
  }

  v8::Local<v8::Value> toRangeError()
  {
    return v8::Exception::RangeError(v8::String::New(mMessage.c_str()));
  }

  v8::Local<v8::Value> toReferenceError()
  {
    return v8::Exception::ReferenceError(v8::String::New(mMessage.c_str()));
  }

  v8::Local<v8::Value> toSyntaxError()
  {
    return v8::Exception::SyntaxError(v8::String::New(mMessage.c_str()));
  }

  v8::Local<v8::Value> toGenericError()
  {
    return v8::Exception::Error(v8::String::New(mMessage.c_str()));
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

  static v8::Handle<v8::Value> throwRtError(rtError err, const char* format, ...) RT_PRINTF_FORMAT(2, 3)
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

    return v8::ThrowException(v8::Exception::Error(v8::String::New(buff)));
  }

  TRef mWrappedObject;
};

rtValue js2rt(const v8::Handle<v8::Value>& val, rtWrapperError* error);

v8::Handle<v8::Value> rt2js(const rtValue& val);


void rtWrapperSceneUpdateEnter();
void rtWrapperSceneUpdateExit();

#endif

