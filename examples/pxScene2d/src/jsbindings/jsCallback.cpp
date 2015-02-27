#include "jsCallback.h"
#include "rtWrapperUtils.h"

jsCallback::jsCallback(v8::Isolate* isolate)
  : mIsolate(isolate)
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

  Handle<Value>* args = ctx->makeArgs();

  // TODO: This context is almost certainly wrong!!!
  Local<Function> func= ctx->mFunctionLookup->lookup();
  Local<Context> context = func->CreationContext();

  TryCatch tryCatch;
  if (!func.IsEmpty())
    func->Call(context->Global(), static_cast<int>(ctx->mArgs.size()), args);

  if (tryCatch.HasCaught())
  {
    String::Utf8Value trace(tryCatch.StackTrace());
    rtLogWarn("%s", *trace);
  }

  delete ctx;
  delete [] args;
}

