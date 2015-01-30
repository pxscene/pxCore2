#include "rtFunctionWrapper.h"
#include "rtWrapperUtils.h"

#include <vector>

static const char* kClassName = "Function";
static Persistent<Function> ctor;

rtFunctionWrapper::rtFunctionWrapper(const rtFunctionRef& ref)
  : rtWrapper(ref)
{
}

rtFunctionWrapper::~rtFunctionWrapper()
{
}

void rtFunctionWrapper::exportPrototype(Handle<Object> exports)
{
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(create);
    tmpl->SetClassName(String::NewSymbol(kClassName));

    // Local<Template> proto = tmpl->PrototypeTemplate();

    Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
    inst->SetInternalFieldCount(1);
    inst->SetCallAsFunctionHandler(call);

    ctor = Persistent<Function>::New(tmpl->GetFunction());
    exports->Set(String::NewSymbol(kClassName), ctor);
}

Handle<Value> rtFunctionWrapper::create(const Arguments& args)
{ 
  if (args.IsConstructCall())
  {
    rtIFunction* p = reinterpret_cast<rtIFunction *>(Local<External>::Cast(args[0])->Value());
    rtFunctionWrapper* wrapper = new rtFunctionWrapper(p);
    wrapper->Wrap(args.This());
    return args.This();
  }
  else
  {
    const int argc = 1;

    v8::HandleScope scope;
    v8::Local<v8::Value> argv[argc] = { args[0] };
    return scope.Close(ctor->NewInstance(argc, argv));
  }
}

Handle<Object> rtFunctionWrapper::createFromFunctionReference(const rtFunctionRef& func)
{
  HandleScope scope;
  Local<Value> argv[1] = { External::New(func.getPtr()) };
  Local<Object> obj = ctor->NewInstance(1, argv);
  return scope.Close(obj);
}

Handle<Value> rtFunctionWrapper::call(const Arguments& args)
{
  HandleScope scope;

  std::vector<rtValue> argList;
  for (int i = 0; i < args.Length(); ++i)
    argList.push_back(js2rt(args[i]));

  rtLogDebug("Invoking function");

  rtValue result;
  rtError err = unwrap(args)->Send(args.Length(), &argList[0], &result);
  if (err != RT_OK)
  {
    rtLogFatal("failed to invoke function: %d", err);
    abort();
  }

  return scope.Close(rt2js(result));
}

