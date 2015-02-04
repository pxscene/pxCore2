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

  rtWrapperError error;

  std::vector<rtValue> argList;
  for (int i = 0; i < args.Length(); ++i)
  {
    argList.push_back(js2rt(args[i], &error));
    if (error.hasError())
      return ThrowException(error.toTypeError());
  }

  rtValue result;
  rtWrapperSceneUpdateEnter();
  rtError err = unwrap(args)->Send(args.Length(), &argList[0], &result);
  rtWrapperSceneUpdateExit();
  if (err != RT_OK)
    rtLogFatal("failed to invoke function: %d", err);

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
  //
  // TODO: Return values are not supported. This class is an rtFunction that wraps
  // a javascript function. If everything is behaving normally, we're running in the
  // context of a native/non-js thread. This is almost certainly the "render" thread.
  // The function is "packed" up and sent off to a javascript thread via the
  // enqueue() on the jsCallback. That means the called is queued with nodejs' event
  // queue. This is required to prevent multiple threads from
  // entering the JS engine. The problem is that the caller can't expect anything in
  // return in the result (last parameter to this function.
  // If you have the current thread wait and then return the result, you'd block this
  // thread until the completion of the javascript function call.
  //
  // Here's an example of how you'd get into this situation. This is a contrived example.
  // No known case exists right now.
  //
  // The closure function below will be wrapped and registered with the rt object layer.
  // If the 'someEvent' is fired, excecution wll arrive here (this code). You'll never see
  // the "return true" because this call returns and the function is run in another
  // thread.
  //
  // This won't work!
  //
  // var foo = ...
  // foo.on('someEvent', function(msg) {
  //    console.log("I'm running in a javascript thread");
  //    return true; // <-- Can't do this
  // });
  //
  jsCallback* callback = jsCallback::create();
  for (int i = 0; i < numArgs; ++i)
    callback->addArg(args[i]);
  callback->setFunctionLookup(new FunctionLookup(this));
  callback->enqueue();

  // result is hard-coded
  if (result)
    *result = rtValue(true);

  return RT_OK;
}

Persistent<Function> jsFunctionWrapper::FunctionLookup::lookup()
{
  return mParent->mFunction;
}

