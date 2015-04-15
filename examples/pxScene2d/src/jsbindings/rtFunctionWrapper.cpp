#include "rtFunctionWrapper.h"
#include "rtWrapperUtils.h"
#include "jsCallback.h"

#include <vector>

static const char* kClassName = "Function";
static Persistent<Function> ctor;

class ResolverFunction : public rtAbstractFunction
{
public:
  ResolverFunction(Isolate* isolate, Local<Promise::Resolver>& resolver)
    : rtAbstractFunction()
    , mResolver(isolate, Handle<Promise::Resolver>::Cast(resolver))
    , mIsolate(isolate)
  {
  }

  virtual ~ResolverFunction()
  {
  }

protected:
  Persistent<Promise::Resolver> mResolver;
  Isolate* mIsolate;
};

class Resolve : public ResolverFunction
{
public:
  Resolve(Isolate*isolate, Local<Promise::Resolver>& resolver)
    : ResolverFunction(isolate, resolver) { }

  virtual ~Resolve() { }

  virtual rtError Send(int numArgs, const rtValue* args, rtValue* /*result*/)
  {
    rtLogInfo("resolving promise");
    Handle<Value> arg;
    if (numArgs && args)
      arg = rt2js(mIsolate, args[0]);

    Local<Promise::Resolver> resolver = PersistentToLocal(mIsolate, mResolver);
    resolver->Resolve(arg);

    return RT_OK;
  }
};

class Reject : public ResolverFunction
{
public:
  Reject(Isolate* isolate, Local<Promise::Resolver>& resolver)
    : ResolverFunction(isolate, resolver) { }

  virtual ~Reject() { }

  virtual rtError Send(int numArgs, const rtValue* args, rtValue* /*result*/)
  {
    rtLogInfo("Rejecting promise");
    Handle<Value> arg;
    if (numArgs && args)
      arg = rt2js(mIsolate, args[0]);

    Local<Promise::Resolver> resolver = PersistentToLocal(mIsolate, mResolver);
    resolver->Reject(arg);

    return RT_OK;
  }
};

static bool isPromise(const rtValue& v)
{
  if (v.getType() != RT_rtObjectRefType)
    return false;

  rtObjectRef ref = v.toObject();
  if (!ref)
    return false;

  rtString desc;
  rtError err = ref.sendReturns<rtString>("description", desc);
  return err == RT_OK && strcmp(desc.cString(), "rtPromise") == 0;
}

rtFunctionWrapper::rtFunctionWrapper(const rtFunctionRef& ref)
  : rtWrapper(ref)
{
}

rtFunctionWrapper::~rtFunctionWrapper()
{
}

void rtFunctionWrapper::exportPrototype(Isolate* isolate, Handle<Object> exports)
{
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, create);
  tmpl->SetClassName(String::NewFromUtf8(isolate, kClassName));

  Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
  inst->SetInternalFieldCount(1);
  inst->SetCallAsFunctionHandler(call);

  ctor.Reset(isolate, tmpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, kClassName), tmpl->GetFunction());
}

void rtFunctionWrapper::create(const FunctionCallbackInfo<Value>& args)
{ 
  assert(args.IsConstructCall());

  HandleScope scope(args.GetIsolate());
  rtIFunction* func = static_cast<rtIFunction *>(args[0].As<External>()->Value());
  rtFunctionWrapper* wrapper = new rtFunctionWrapper(func);
  wrapper->Wrap(args.This());
}

Handle<Object> rtFunctionWrapper::createFromFunctionReference(Isolate* isolate, const rtFunctionRef& func)
{
  EscapableHandleScope scope(isolate);
  Local<Value> argv[1] = 
  {
    External::New(isolate, func.getPtr()) 
  };
  Local<Function> c = PersistentToLocal(isolate, ctor);
  return scope.Escape(c->NewInstance(1, argv));
}

void rtFunctionWrapper::call(const FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = args.GetIsolate();

  HandleScope scope(isolate);

  rtWrapperError error;

  std::vector<rtValue> argList;
  for (int i = 0; i < args.Length(); ++i)
  {
    argList.push_back(js2rt(isolate, args[i], &error));
    if (error.hasError())
      isolate->ThrowException(error.toTypeError(isolate));
  }

  rtValue result;
  rtWrapperSceneUpdateEnter();
  rtError err = unwrap(args)->Send(args.Length(), &argList[0], &result);
  rtWrapperSceneUpdateExit();
  if (err != RT_OK)
    return throwRtError(isolate, err, "failed to invoke function");

  if (isPromise(result))
  {
    Local<Promise::Resolver> resolver = Promise::Resolver::New(isolate);

    rtFunctionRef resolve(new Resolve(isolate, resolver));
    rtFunctionRef reject(new Reject(isolate, resolver));
    rtObjectRef newPromise;

    rtObjectRef promise = result.toObject();
    rtError err = promise.send("then", resolve, reject, newPromise);
    if (err != RT_OK)
      return throwRtError(isolate, err, "failed to register for promise callback");
    else
      args.GetReturnValue().Set(resolver->GetPromise());
  }
  else
  {
    args.GetReturnValue().Set(rt2js(isolate, result));
  }
}

jsFunctionWrapper::jsFunctionWrapper(Isolate* isolate, const Handle<Value>& val)
  : mRefCount(0)
  , mFunction(isolate, Handle<Function>::Cast(val))
  , mIsolate(isolate)
{
  assert(val->IsFunction());
}

jsFunctionWrapper::~jsFunctionWrapper()
{

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
  jsCallback* callback = jsCallback::create(mIsolate);
  for (int i = 0; i < numArgs; ++i)
    callback->addArg(args[i]);
  callback->setFunctionLookup(new FunctionLookup(this));
  callback->enqueue();

  // result is hard-coded
  if (result)
    *result = rtValue(true);

  return RT_OK;
}

Local<Function> jsFunctionWrapper::FunctionLookup::lookup()
{
  return PersistentToLocal(mParent->mIsolate, mParent->mFunction);
}

