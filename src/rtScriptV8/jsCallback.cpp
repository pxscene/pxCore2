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

#include "jsCallback.h"
#include "rtWrapperUtils.h"


using namespace v8;

namespace rtScriptV8NodeUtils
{

jsCallback::jsCallback(v8::Local<v8::Context>& ctx)
  : mReq()
  , mFunctionLookup(NULL)
  , mIsolate(ctx->GetIsolate())
  , mCompletionFunc(NULL)
  , mCompletionContext(NULL)
{
  mReq.data = this;

  mContext.Reset(mIsolate, ctx);
}

jsCallback::~jsCallback()
{
  mContext.Reset();
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

jsCallback* jsCallback::create(v8::Local<v8::Context>& ctx)
{
  return new jsCallback(ctx);
}

jsCallback* jsCallback::addArg(const rtValue& val)
{
  mArgs.push_back(val);
  return this;
}

Handle<Value>* jsCallback::makeArgs(Local<Context>& ctx)
{
  Handle<Value>* args = new Handle<Value>[mArgs.size()];

  for (size_t i = 0; i < mArgs.size(); ++i)
  {
    args[i] = rt2js(ctx, mArgs[i]);
  }

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
  {
    ctx->mCompletionFunc(ctx->mCompletionContext, ret);
  }

  delete ctx;
}

rtValue jsCallback::run()
{
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope handle_scope(mIsolate);

  Local<Context> ctx = PersistentToLocal(mIsolate, mContext);
  Handle<Value>* args = this->makeArgs(ctx);
  Local<Function> func = this->mFunctionLookup->lookup(ctx);

  assert(!func.IsEmpty());
  assert(!func->IsUndefined());

  // This is really nice debugging
  #if 0
  Local<String> s = func->ToString();
  String::Utf8Value v(s);
  rtLogInfo("FUNC: %s", *v);
  #endif

  Local<Context> context = func->CreationContext();
  Context::Scope contextScope(context);

  Local<Value> val;

#if defined ENABLE_NODE_V_6_9 || defined RTSCRIPT_SUPPORT_V8
  TryCatch tryCatch(mIsolate);
#else
  TryCatch tryCatch;
#endif //ENABLE_NODE_V_6_9 
  if (!func.IsEmpty())
  {
    // TODO: check that first arg. Is that 'this' why are we using context->Global()?
    val = func->Call(context->Global(), static_cast<int>(this->mArgs.size()), args);
  }

  delete [] args;

  rtValue returnValue;
  if (tryCatch.HasCaught())
  {
    String::Utf8Value trace(tryCatch.StackTrace());
    rtLogWarn("%s", *trace);
  }
  else
  {
    rtWrapperError error;
    returnValue = js2rt(context, val, &error);
  }

  return returnValue;
}

} // namespace
