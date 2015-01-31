#include "rtFunctionWrapper.h"
#include "rtWrapperUtils.h"
#include "jsCallback.h"

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

    HandleScope scope;
    Local<Value> argv[argc] = { args[0] };
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
  rtWrapperSceneUpdateEnter();
  rtError err = unwrap(args)->Send(args.Length(), &argList[0], &result);
  rtWrapperSceneUpdateExit();
  if (err != RT_OK)
  {
    rtLogFatal("failed to invoke function: %d", err);
    abort();
  }

  return scope.Close(rt2js(result));
}


jsFunctionWrapper::jsFunctionWrapper(const Handle<Value>& val)
  : mRefCount(0)
{
  assert(val->IsFunction());
  mFunction = Persistent<Function>::New(Handle<Function>::Cast(val));
}

jsFunctionWrapper::~jsFunctionWrapper()
{
  mFunction.Dispose();
}

unsigned long jsFunctionWrapper::AddRef()
{
  return rtAtomicInc(&mRefCount);
}

unsigned long jsFunctionWrapper::Release()
{
  unsigned long l = rtAtomicDec(&mRefCount);
  if (l == 0) delete this;
  return l;
}

rtError jsFunctionWrapper::Send(int numArgs, const rtValue* args, rtValue* result)
{
  jsCallback* callback = jsCallback::create();
  for (int i = 0; i < numArgs; ++i)
    callback->addArg(args[i]);
  callback->setFunctionLookup(new FunctionLookup(this));
  callback->enqueue();
  return RT_OK;
}

Persistent<Function> jsFunctionWrapper::FunctionLookup::lookup()
{
  return mParent->mFunction;
}


