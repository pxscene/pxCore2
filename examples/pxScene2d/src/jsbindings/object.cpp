#include "px.h"
#include "pxScene2d.h"

namespace rt
{
  void Object::Inherit(v8::Local<v8::FunctionTemplate> derived)
  {
    v8::Local<v8::Template> proto = derived->PrototypeTemplate();
    proto->Set(v8::String::NewSymbol("get"), v8::FunctionTemplate::New(Get)->GetFunction());
  }

  v8::Handle<v8::Value> Object::Get(const v8::Arguments& args)
  {
    v8::HandleScope scope;
    v8::Local<v8::Object> self = args.This();
    v8::String::Utf8Value propertyName(args[0]->ToString());

    rt::Object* obj = node::ObjectWrap::Unwrap<rt::Object>(self);

    rtValue value(RT_OK);
    rtError err = obj->m_obj->Get(*propertyName, &value);
    if (err == RT_OK)
      return scope.Close(rt2js(value, obj->m_obj, self));
      
    return scope.Close(v8::Undefined());
  }

  v8::Handle<v8::Value> Object::Set(const v8::Arguments& args)
  {
    v8::HandleScope scope;
    v8::Local<v8::Object> self = args.This();
    v8::String::Utf8Value propertyName(args[0]->ToString());

    rt::Object* obj = node::ObjectWrap::Unwrap<rt::Object>(self);
    
    rtValue value = js2rt(args[1]);
    rtError err = obj->m_obj->Set(*propertyName, &value);

    return scope.Close(v8::Undefined());
  }

  v8::Handle<v8::Value> rt2js(const rtValue& v, rtObject* rt, const v8::Handle<v8::Object>& js)
  {
    switch (v.getType())
    {
      case RT_int32_tType:
        return v8::Integer::New(v.toInt32());
        break;
      case RT_uint32_tType:
        return v8::Integer::NewFromUnsigned(v.toUInt32());
        break;
      case RT_int64_tType:
        return v8::Number::New(v.toDouble());
        break;
      case RT_uint64_tType:
        return v8::Number::New(v.toDouble());
        break;
      case RT_functionType:
        return rt::Function::New(v.toFunction());
        break;
      default:
        // TODO: FAIL
        fprintf(stderr, "unsupported rt type: %c\n", v.getType());
        assert(false);
        break;
    }

    return v8::Undefined();
  }

  rtValue js2rt(const v8::Handle<v8::Value>& val)
  {
    fprintf(stderr, "Implemente me: %s:%d\n", __FILE__, __LINE__);
    assert(false);
    return rtValue(0);
  }
}
