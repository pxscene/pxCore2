/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "rtFunctionWrapper.h"
#include "rtWrapperUtils.h"
#include "jsCallback.h"

#include <vector>
#ifdef RUNINMAIN
extern bool gIsPumpingJavaScript;
#endif
using namespace v8;

namespace rtScriptV8NodeUtils
{

static const char* kClassName = "Function";
static Persistent<v8::Function> ctor;
std::hash<std::string> hashFn;

static void jsFunctionCompletionHandler(void* argp, rtValue const& result)
{
  jsFunctionWrapper* wrapper = reinterpret_cast<jsFunctionWrapper *>(argp);
  wrapper->signal(result);
}

rtResolverFunction::rtResolverFunction(Disposition disp, v8::Local<v8::Context>& ctx, Local<v8::Promise::Resolver>& resolver)
  : rtAbstractFunction()
  , mDisposition(disp)
  , mResolver(ctx->GetIsolate(), resolver)
  , mContext(ctx->GetIsolate(), ctx)
  , mIsolate(ctx->GetIsolate())
  , mReq()
{
}

rtResolverFunction::~rtResolverFunction()
{
  rtLogDebug("~rtResolverFunction");
  mResolver.Reset();
}

rtError rtResolverFunction::Send(int numArgs, const rtValue* args, rtValue* /*result*/)
{
  AsyncContext* ctx = new AsyncContext();

  // keep current object alive while we enqueue this request
  ctx->resolverFunc = rtFunctionRef(this);

  for (int i = 0; i < numArgs; ++i)
  {
    ctx->args.push_back(args[i]);
  }

  mReq.data = ctx;
  uv_queue_work(uv_default_loop(), &mReq, &workCallback, &afterWorkCallback);
  return RT_OK;
}

void rtResolverFunction::workCallback(uv_work_t* /*req */)
{
  // empty
}

void rtResolverFunction::afterWorkCallback(uv_work_t* req, int /* status */)
{
  AsyncContext* ctx = reinterpret_cast<AsyncContext*>(req->data);
  rtResolverFunction* resolverFunc = static_cast<rtResolverFunction *>(ctx->resolverFunc.getPtr());

  assert(ctx->args.size() < 2);
//  assert(Isolate::GetCurrent() == resolverFunc->mIsolate);

  Locker                locker(resolverFunc->mIsolate);
  Isolate::Scope isolate_scope(resolverFunc->mIsolate);
  HandleScope     handle_scope(resolverFunc->mIsolate);

  Local<Context> creationContext = PersistentToLocal(resolverFunc->mIsolate, resolverFunc->mContext);
  Context::Scope contextScope(creationContext);

//DEBUG
  // uint32_t id = GetContextId(creationContext);
  // rtLogInfo("rtResolverFunction:: >>> Send:%u", id);
//DEBUG

  Handle<Value> value;
  if (ctx->args.size() > 0)
  {
    value = rt2js(creationContext, ctx->args[0]);
  }

  Local<Promise::Resolver> resolver = PersistentToLocal(resolverFunc->mIsolate, resolverFunc->mResolver);
  Local<Context> local_context = resolver->CreationContext();
  Context::Scope context_scope(local_context);

#if defined ENABLE_NODE_V_6_9 || defined RTSCRIPT_SUPPORT_V8
  TryCatch tryCatch(resolverFunc->mIsolate);
#else
  TryCatch tryCatch;
#endif //ENABLE_NODE_V_6_9
  if (resolverFunc->mDisposition == DispositionResolve)
  {
    resolver->Resolve(value);
  }
  else
  {
    resolver->Reject(value);
  }

  if (tryCatch.HasCaught())
  {
    String::Utf8Value trace(tryCatch.StackTrace());
    rtLogWarn("Error resolving promise");
    rtLogWarn("%s", *trace);
  }
#ifdef RUNINMAIN
  if (false == gIsPumpingJavaScript)
  {
#endif
  resolverFunc->mIsolate->RunMicrotasks();
#ifdef RUNINMAIN
  }
#endif
  delete ctx;
}

rtFunctionWrapper::rtFunctionWrapper(const rtFunctionRef& ref)
  : rtWrapper(ref)
{
}

rtFunctionWrapper::~rtFunctionWrapper()
{
}

void rtFunctionWrapper::destroyPrototype()
{
  if( !ctor.IsEmpty() )
  {
    // TODO: THIS LEAKS... need to free obj within persistent

    //  rtFunctionWrapper *obj = V8::Utils::OpenPersistent(ctor);

    ctor.ClearWeak();
    ctor.Reset();
  }
}

void rtFunctionWrapper::exportPrototype(Isolate* isolate, Handle<Object> exports)
{
  Locker                locker(isolate);
  Isolate::Scope isolate_scope(isolate);
  HandleScope     handle_scope(isolate);  // Create a stack-allocated handle scope.

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

  // Local<Context> c1 = args.GetIsolate()->GetCurrentContext();
  // Local<Context> c2 = args.GetIsolate()->GetCallingContext();
  // Local<Context> c3 = args.GetIsolate()->GetEnteredContext();
  // rtLogInfo("current:%u calling:%u entered:%u", GetContextId(c1), GetContextId(c2),
  //     GetContextId(c3));

  rtIFunction* func = static_cast<rtIFunction *>(args[0].As<External>()->Value());
  rtFunctionWrapper* wrapper = new rtFunctionWrapper(func);
  wrapper->Wrap(args.This());
}

#if defined ENABLE_NODE_V_6_9 || defined RTSCRIPT_SUPPORT_V8
Handle<Object> rtFunctionWrapper::createFromFunctionReference(v8::Local<v8::Context>& ctx, Isolate* isolate, const rtFunctionRef& func)
#else
Handle<Object> rtFunctionWrapper::createFromFunctionReference(Isolate* isolate, const rtFunctionRef& func)
#endif
{
  Locker                       locker(isolate);
  Isolate::Scope        isolate_scope(isolate);
  EscapableHandleScope          scope(isolate);

  Local<Value> argv[1] =
  {
    External::New(isolate, func.getPtr())
  };

  Local<Function> c = PersistentToLocal(isolate, ctor);
#ifdef ENABLE_NODE_V_6_9
  return scope.Escape((c->NewInstance(ctx, 1, argv)).FromMaybe(Local<Object>()));
#else
  return scope.Escape(c->NewInstance(1, argv));
#endif
}

void rtFunctionWrapper::call(const FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = args.GetIsolate();

  Locker                  locker(isolate);
  Isolate::Scope   isolate_scope(isolate);
  HandleScope       handle_scope(isolate);  // Create a stack-allocated handle scope.

  // Local<Context> curr = isolate->GetCurrentContext();

  rtWrapperError error;

  #if 0
  Local<String> s = args.This()->ToString();
  String::Utf8Value v(s);
  rtLogInfo("call:%s", *v);
  #endif

  Local<Context> ctx = args.This()->CreationContext();
  // rtLogInfo("id: %u", GetContextId(ctx));
  Context::Scope contextScope(ctx);

  std::vector<rtValue> argList;
  for (int i = 0; i < args.Length(); ++i)
  {
    argList.push_back(js2rt(ctx, args[i], &error));
    if (error.hasError())
      isolate->ThrowException(error.toTypeError(isolate));
  }

  rtValue result;
  rtWrapperSceneUpdateEnter();
  rtError err = unwrap(args)->Send(args.Length(), &argList[0], &result);

  if (err != RT_OK)
  {
    rtWrapperSceneUpdateExit();
    return throwRtError(isolate, err, "failed to invoke function");
  }

  if (rtIsPromise(result))
  {
    Local<Promise::Resolver> resolver = Promise::Resolver::New(isolate);

    rtFunctionRef resolve(new rtResolverFunction(rtResolverFunction::DispositionResolve, ctx, resolver));
    rtFunctionRef reject(new rtResolverFunction(rtResolverFunction::DispositionReject, ctx, resolver));

    rtObjectRef newPromise;
    rtObjectRef promise = result.toObject();

    Local<Object> jsPromise = resolver->GetPromise();
    HandleMap::addWeakReference(isolate, promise, jsPromise);

    rtError err = promise.send("then", resolve, reject, newPromise);

    // must hold this lock to prevent promise from resolving internally before we
    // actually register our function callbacks.
    rtWrapperSceneUpdateExit();

    if (err != RT_OK)
      return throwRtError(isolate, err, "failed to register for promise callback");
    else
      args.GetReturnValue().Set(jsPromise);
  }
  else
  {
    rtWrapperSceneUpdateExit();
    args.GetReturnValue().Set(rt2js(ctx, result));
  }
}

jsFunctionWrapper::jsFunctionWrapper(Local<Context>& ctx, const Handle<Value>& val)
  : mRefCount(0)
  , mFunction(ctx->GetIsolate(), Handle<Function>::Cast(val))
  , mComplete(false)
  , mTeardownThreadingPrimitives(false)
  , mHash(-1)
{
  v8::String::Utf8Value fn(Handle<Function>::Cast(val)->ToString());
  if (NULL != *fn) { 
    mHash = hashFn(*fn);
  }
  mIsolate = ctx->GetIsolate();
  mContext.Reset(ctx->GetIsolate(), ctx);
  assert(val->IsFunction());
}

jsFunctionWrapper::~jsFunctionWrapper()
{
  mFunction.Reset();
  mContext.Reset();

  if (mTeardownThreadingPrimitives)
  {
#ifndef USE_STD_THREADS
    pthread_mutex_destroy(&mMutex);
    pthread_cond_destroy(&mCond);
#endif
  }
  mHash = -1;
}

void jsFunctionWrapper::setupSynchronousWait()
{
  mTeardownThreadingPrimitives = true;
#ifndef USE_STD_THREADS
  pthread_mutex_init(&mMutex, NULL);
  pthread_cond_init(&mCond, NULL);
#endif
#ifdef USE_STD_THREADS
  std::unique_lock<std::mutex> lock(mMutex);
  mComplete = false;
#else
  pthread_mutex_lock(&mMutex);
  mComplete = false;
  pthread_mutex_unlock(&mMutex);
#endif
}

void jsFunctionWrapper::signal(rtValue const& returnValue)
{
#ifdef USE_STD_THREADS
  std::unique_lock<std::mutex> lock(mMutex);
#else
  pthread_mutex_lock(&mMutex);
#endif

  mComplete = true;
  mReturnValue = returnValue;

#ifdef USE_STD_THREADS
  mCond.notify_one();
#else
  pthread_mutex_unlock(&mMutex);
  pthread_cond_signal(&mCond);
#endif
}

rtValue jsFunctionWrapper::wait()
{
#ifdef USE_STD_THREADS
  std::unique_lock<std::mutex> lock(mMutex);
  mCond.wait(lock, [&] { return mComplete; });
#else
  pthread_mutex_lock(&mMutex);
  while (!mComplete)
    pthread_cond_wait(&mCond, &mMutex);
  pthread_mutex_unlock(&mMutex);
#endif
  return mReturnValue;
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

  Locker locker(mIsolate);
  HandleScope handleScope(mIsolate);
  Local<Context> ctx = PersistentToLocal(mIsolate, mContext);
  Context::Scope contextScope(ctx);

  jsCallback* callback = jsCallback::create(ctx);
  for (int i = 0; i < numArgs; ++i)
    callback->addArg(args[i]);

  callback->setFunctionLookup(new FunctionLookup(this));

  if (result) // wants result run synchronously
  {
    if (rtIsMainThreadNode()) // main thread run now
    {
      *result = callback->run();
      delete callback;
    }
    else // queue and wait
    {
      setupSynchronousWait();
      callback->registerForCompletion(jsFunctionCompletionHandler, this);
      callback->enqueue();

      // don't block render thread while waiting for callback to complete
      rtWrapperSceneUnlocker unlocker;

      // !CLF: When/why was this wait() introduced?  Need fix for multi-thread solution
     // *result = wait();
    }
  }
  else // just queue
  {
    callback->enqueue();
  }

  return RT_OK;
}

Local<Function> jsFunctionWrapper::FunctionLookup::lookup(v8::Local<v8::Context>& ctx)
{
  jsFunctionWrapper* parent = (jsFunctionWrapper*)mParent.getPtr();
  EscapableHandleScope handleScope(parent->mIsolate);
  Context::Scope contextScope(ctx);
  Local<Function> func = PersistentToLocal(parent->mIsolate, parent->mFunction);
  return handleScope.Escape(func);
}

} // namespace
