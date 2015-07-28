#include "jsCallback.h"
#include "rtWrapperUtils.h"

jsCallback::jsCallback(v8::Isolate* isolate)
  : mIsolate(isolate)
  , mCompletionFunc(NULL)
  , mCompletionContext(NULL)
{
  mReq.data = this;
  mFunctionLookup = NULL;
}

jsCallback::~jsCallback()
{
  delete mFunctionLookup;
}

void jsCallback::enqueue()
{
  uv_queue_work(uv_default_loop(), &mReq, &work, &doCallback);
}

void jsCallback::registerForCompletion(jsCallbackCompletionFunc callback, void* argp)
{
  mCompletionFunc = callback;
  mCompletionContext = argp;
}

void jsCallback::work(uv_work_t* /* req */)
{
}

jsCallback* jsCallback::create(v8::Isolate* isolate)
{
  return new jsCallback(isolate);
}

jsCallback* jsCallback::addArg(const rtValue& val)
{
  mArgs.push_back(val);
  return this;
}

Handle<Value>* jsCallback::makeArgs()
{
  Handle<Value>* args = new Handle<Value>[mArgs.size()];
  for (size_t i = 0; i < mArgs.size(); ++i)
    args[i] = rt2js(mIsolate, mArgs[i]);
  return args;
}

jsCallback* jsCallback::setFunctionLookup(jsIFunctionLookup* functionLookup)
{
  mFunctionLookup = functionLookup;
  return this;
}

void jsCallback::doCallback(uv_work_t* req, int /* status */)
{
  jsCallback* ctx = reinterpret_cast<jsCallback *>(req->data);
  assert(ctx != NULL);
  assert(ctx->mFunctionLookup != NULL);

  rtValue ret  = ctx->run();
  if (ctx->mCompletionFunc)
    ctx->mCompletionFunc(ctx->mCompletionContext, ret);

  delete ctx;
}

rtValue jsCallback::run()
{
  Handle<Value>* args = this->makeArgs();

  // TODO: This context is almost certainly wrong!!!
  Local<Function> func = this->mFunctionLookup->lookup();
  assert(!func.IsEmpty());
  assert(!func->IsUndefined());

  // This is really nice debugging
  #if 0
  Local<String> s = func->ToString();
  String::Utf8Value v(s);
  rtLogInfo("FUNC: %s", *v);
  #endif

  Local<Context> context = func->CreationContext();

  Local<Value> val;

  TryCatch tryCatch;
  if (!func.IsEmpty())
    val = func->Call(context->Global(), static_cast<int>(this->mArgs.size()), args);

  if (tryCatch.HasCaught())
  {
    String::Utf8Value trace(tryCatch.StackTrace());
    rtLogWarn("%s", *trace);
  }

  delete [] args;

  rtWrapperError error;
  return js2rt(context->GetIsolate(), val, &error);
}

