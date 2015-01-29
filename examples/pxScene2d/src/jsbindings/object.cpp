#include "px.h"
#include "pxScene2d.h"

using namespace v8;

namespace
{
  const char* kClassName = "Object";

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
  v8::Persistent<v8::Function> rt::Object::m_ctor;

  void rt::Object::Export(v8::Handle<v8::Object> exports)
  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(New);
    tmpl->SetClassName(String::NewSymbol(kClassName));

    Local<Template> proto = tmpl->PrototypeTemplate();
    proto->Set(String::NewSymbol("get"), FunctionTemplate::New(Get)->GetFunction());
    proto->Set(String::NewSymbol("set"), FunctionTemplate::New(Set)->GetFunction());
    proto->Set(String::NewSymbol("send"), FunctionTemplate::New(Send)->GetFunction());

    Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
    inst->SetInternalFieldCount(1);
    inst->SetNamedPropertyHandler(&GetProperty, &SetProperty);

    m_ctor = Persistent<v8::Function>::New(tmpl->GetFunction());
    exports->Set(String::NewSymbol(kClassName), m_ctor);
  }

  void rt::Object::Inherit(Local<FunctionTemplate> derived)
  {
    Local<Template> proto = derived->PrototypeTemplate();
    proto->Set(String::NewSymbol("get"), FunctionTemplate::New(Get)->GetFunction());
    proto->Set(String::NewSymbol("set"), FunctionTemplate::New(Set)->GetFunction());
    proto->Set(String::NewSymbol("send"), FunctionTemplate::New(Send)->GetFunction());
  }

  Handle<Value> rt::Object::GetProperty(Local<String> name, const AccessorInfo& info)
  {
    rtString propertyName = toString(name);
    rtLogDebug("getting property: %s", propertyName.cString());

    rtValue value;
    rtError err = unwrap(info)->Get(propertyName.cString(), &value);
    if (err != RT_OK)
      return Handle<Value>(Undefined());

    return rt2js(value);
  }

  Handle<Value> rt::Object::SetProperty(Local<String> name, Local<Value> val, const AccessorInfo& info)
  {
    rtString propertyName = toString(name);

    rtValue value = js2rt(val);
    rtError err = unwrap(info)->Set(propertyName.cString(), &value);
    return err == RT_OK
      ? val
      : Handle<Value>();
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

  Handle<Value> rt::Object::Send(const Arguments& args)
  {
    HandleScope scope;

    rtString name = toString(args[0]->ToString());

    rtValue method;
    rtError err = unwrap(args)->Get(name.cString(), &method);
    if (err == RT_OK)
    {
      rtLog("invoke function:%s\n", name.cString());

      std::vector<rtValue> argList;
      for (int i = 1; i < args.Length(); ++i)
        argList.push_back(js2rt(args[i]));

      rtValue value;
      rtFunctionRef func = method.toFunction();
      err = func->Send(static_cast<int>(argList.size()), &argList[0], &value);
      if (err == RT_OK)
        return scope.Close(rt2js(value));
    }

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

  Handle<v8::Object> rt::Object::New(const rtObjectRef& rtobj)
  {
    HandleScope scope;
    Local<Value> argv[1] = { External::New(rtobj.getPtr()) };
    Local<v8::Object> obj = m_ctor->NewInstance(1, argv);
    return scope.Close(obj);
  }

  Handle<Value> rt::Object::New(const Arguments& args)
  { 
    if (args.IsConstructCall())
    {
      rtObject* rtobj = reinterpret_cast<rtObject*>(Local<External>::Cast(args[0])->Value());
      rt::Object* obj = new rt::Object(rtobj);
      obj->Wrap(args.This());
      return args.This();
    }
    else
    {
      // invoked as Scene2d()
      const int argc = 1;

      HandleScope scope;
      Local<Value> argv[argc] = { args[0] };
      return scope.Close(m_ctor->NewInstance(argc, argv));
    }
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
      case RT_rtObjectRefType:
        return rt::Object::New(v.toObject());
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
