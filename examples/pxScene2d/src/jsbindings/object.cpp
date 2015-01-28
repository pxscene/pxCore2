#include "px.h"
#include "pxScene2d.h"

using namespace v8;

namespace
{
  rtString toString(const v8::Handle<v8::Object>& obj)
  {
    v8::String::Utf8Value utf(obj->ToString());
    return rtString(*utf);
  }

  rtString toString(const v8::Handle<v8::Value>& val)
  {
    v8::String::Utf8Value utf(val->ToString());
    return rtString(*utf);
  }

  rtString toString(const v8::Local<v8::String>& s)
  {
    v8::String::Utf8Value utf(s);
    return rtString(*utf);
  }
}

namespace rt
{
  void rt::Object::Inherit(Local<FunctionTemplate> derived)
  {
    Local<Template> proto = derived->PrototypeTemplate();
    proto->Set(String::NewSymbol("get"), FunctionTemplate::New(Get)->GetFunction());
  }

  Handle<Value> rt::Object::GetProperty(Local<String> name, const AccessorInfo& info)
  {
    rtString propertyName = toString(name);

    rtValue value;
    rtError err = unwrap(info)->Get(propertyName.cString(), &value);
    if (err != RT_OK)
      return Handle<Value>(Undefined());

    return rt2js(value);
  }

  void rt::Object::SetProperty(Local<String> name, Local<Value> val, const AccessorInfo& info)
  {
    rtString propertyName = toString(name);

    rtValue value = js2rt(val);
    rtError err = unwrap(info)->Set(propertyName.cString(), &value);
    if (err != RT_OK)
      PX_THROW(Error, "failed to set property %s. %lu", propertyName.cString(), err);
  }

  Handle<Value> rt::Object::Get(const Arguments& args)
  {
    HandleScope scope;

    rtString propertyName = toString(args[0]->ToString());

    rtValue value(RT_OK);
    rtError err = unwrap(args)->Get(propertyName.cString(), &value);
    if (err == RT_OK)
      return scope.Close(rt2js(value)); 
      
    return scope.Close(Undefined());
  }

  Handle<Value> rt::Object::Set(const Arguments& args)
  {
    HandleScope scope;

    rtString propertyName = toString(args[0]->ToString());

    rtValue value = js2rt(args[1]);
    rtError err = unwrap(args)->Set(propertyName.cString(), &value);
    if (err != RT_OK)
      PX_THROW(Error, "failed to set property %s. %lu", propertyName.cString(), err);

    return scope.Close(Undefined());
  }

  Handle<Value> rt2js(const rtValue& v)
  {
    switch (v.getType())
    {
      case RT_int32_tType:
        return Integer::New(v.toInt32());
        break;
      case RT_uint32_tType:
        return Integer::NewFromUnsigned(v.toUInt32());
        break;
      case RT_int64_tType:
        return Number::New(v.toDouble());
        break;
      case RT_uint64_tType:
        return Number::New(v.toDouble());
        break;
      case RT_functionType:
        return rt::Function::New(v.toFunction());
        break;
      case RT_rtStringType:
        {
          rtString s = v.toString();
          return String::New(s.cString(), s.length());
        }
        break;
      default:
        fprintf(stderr, "unsupported rtValue (%c)  to javascript conversion", v.getType());
        assert(false);
        break;
    }

    return Undefined();
  }

  rtValue js2rt(const Handle<Value>& val)
  {
    if (val->IsUndefined()) { return rtValue((void *)0); }
    if (val->IsNull())      { return rtValue((char *)0); }
    if (val->IsString())    { return toString(val); }
    if (val->IsFunction())  { assert(false); return rtValue(0); }
    if (val->IsArray())     { assert(false); return rtValue(0); }
    if (val->IsObject())    { assert(false); return rtValue(0); }
    if (val->IsBoolean())   { return rtValue(val->BooleanValue()); }
    if (val->IsNumber())    { return rtValue(val->NumberValue()); }
    if (val->IsInt32())     { return rtValue(val->Int32Value()); }
    if (val->IsUint32())    { return rtValue(val->Uint32Value()); }

    fprintf(stderr, "unsupported javasciprt -> rtValue type conversion");
    assert(false);

    return rtValue(0);
  }
}
