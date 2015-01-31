#include "jsCallback.h"
#include "rtWrapperUtils.h"

jsCallback::jsCallback()
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

void jsCallback::work(uv_work_t* req)
{
}

jsCallback* jsCallback::create()
{ 
  return new jsCallback();
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
    args[i] = rt2js(mArgs[i]);
  return args;
}

jsCallback* jsCallback::setFunctionLookup(jsIFunctionLookup* functionLookup)
{ 
  mFunctionLookup = functionLookup;
  return this; 
}

void jsCallback::doCallback(uv_work_t* req, int status)
{
  jsCallback* ctx = reinterpret_cast<jsCallback *>(req->data);
  assert(ctx != NULL);
  assert(ctx->mFunctionLookup != NULL);

  Handle<Value>* args = ctx->makeArgs();

  // TODO: Should this be Local<Function>? 
  Persistent<Function> callbackFunction = ctx->mFunctionLookup->lookup();
  if (!callbackFunction.IsEmpty())
    callbackFunction->Call(Context::GetCurrent()->Global(), static_cast<int>(ctx->mArgs.size()), args);

  delete ctx;
  delete [] args;
}

