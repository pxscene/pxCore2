#include "rtObjectWrapper.h"
#include "rtWrapperUtils.h"

#include <rtLog.h>

using namespace v8;

static const char* kClassName = "Object";
static Persistent<Function> ctor;

rtObjectWrapper::rtObjectWrapper(const rtObjectRef& ref)
  : rtWrapper(ref)
{

}

rtObjectWrapper::~rtObjectWrapper()
{

}

void rtObjectWrapper::exportPrototype(Handle<Object> exports)
{
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(create);
  tmpl->SetClassName(String::NewSymbol(kClassName));

  // Local<Template> proto = tmpl->PrototypeTemplate();
  // proto->Set(String::NewSymbol("get"), FunctionTemplate::New(Get)->GetFunction());
  // proto->Set(String::NewSymbol("set"), FunctionTemplate::New(Set)->GetFunction());
  // proto->Set(String::NewSymbol("send"), FunctionTemplate::New(Send)->GetFunction());

  Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
  inst->SetInternalFieldCount(1);
  inst->SetNamedPropertyHandler(&getProperty, &setProperty);

  ctor = Persistent<Function>::New(tmpl->GetFunction());
  exports->Set(String::NewSymbol(kClassName), ctor);
}

Handle<Object> rtObjectWrapper::createFromObjectReference(const rtObjectRef& ref)
{
  HandleScope scope;
  Local<Value> argv[1] = { External::New(ref.getPtr()) };
  Local<Object> obj = ctor->NewInstance(1, argv);
  return scope.Close(obj);
}

rtValue rtObjectWrapper::unwrapObject(const Local<Object>& obj)
{
  return rtValue(unwrap(obj));
}

Handle<Value> rtObjectWrapper::getProperty(
    Local<String> prop,
    const AccessorInfo& info)
{
  rtString propertyName = toString(prop);
  rtLogDebug("getting property: %s", propertyName.cString());

  rtValue value;
  rtWrapperSceneUpdateEnter();
  rtError err = unwrap(info)->Get(propertyName.cString(), &value);
  rtWrapperSceneUpdateExit();
  if (err != RT_OK)
    return Handle<Value>(Undefined());

  return rt2js(value);
}

Handle<Value> rtObjectWrapper::setProperty(
    Local<String> prop,
    Local<Value> val,
    const AccessorInfo& info)
{
  rtString name = toString(prop);
  rtValue value = js2rt(val);

  rtLogDebug("set %s=%s", name.cString(), value.toString().cString());

  rtWrapperSceneUpdateEnter();
  rtError err = unwrap(info)->Set(name.cString(), &value);
  rtWrapperSceneUpdateExit();
  return err == RT_OK
    ? val
    : Handle<Value>();
}

Handle<Value> rtObjectWrapper::create(const Arguments& args)
{ 
  if (args.IsConstructCall())
  {
    rtObject* obj = reinterpret_cast<rtObject*>(Local<External>::Cast(args[0])->Value());
    rtObjectWrapper* wrapper = new rtObjectWrapper(obj);
    wrapper->Wrap(args.This());
    return args.This();
  }
  else
  {
    // invoked as rtObjectWrapper()
    const int argc = 1;

    HandleScope scope;
    Local<Value> argv[argc] = { args[0] };
    return scope.Close(ctor->NewInstance(argc, argv));
  }
}

