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

#ifndef RT_FUNCTION_WRAPPER_H
#define RT_FUNCTION_WRAPPER_H

#include "rtWrapperUtils.h"
#include "jsCallback.h"

#ifdef USE_STD_THREADS
#  include <mutex>
#  include <condition_variable>
#endif

namespace rtScriptV8NodeUtils
{

class rtAbstractFunction : public rtIFunction
{
public:
  virtual unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  virtual unsigned long Release()
  {
    unsigned long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  virtual unsigned long getRefCount() const
  {
    return mRefCount;
  }
protected:
  virtual ~rtAbstractFunction() {}
  unsigned long mRefCount;
};

class rtFunctionWrapper : public rtWrapper<rtFunctionRef, rtFunctionWrapper>
{
public:
  rtFunctionWrapper(const rtFunctionRef& ref);
  virtual ~rtFunctionWrapper();

public:
  static void exportPrototype(v8::Isolate* isolate, v8::Handle<v8::Object> exports);
  static void destroyPrototype();
#if defined ENABLE_NODE_V_6_9 || defined RTSCRIPT_SUPPORT_V8
  static v8::Handle<v8::Object> createFromFunctionReference(v8::Local<v8::Context>& ctx, v8::Isolate* isolate, const rtFunctionRef& func);
#else
  static v8::Handle<v8::Object> createFromFunctionReference(v8::Isolate* isolate, const rtFunctionRef& func);
#endif

private:
  static void create(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void call(const v8::FunctionCallbackInfo<v8::Value>& args);
};

class jsFunctionWrapper : public rtIFunction
{
public:
  jsFunctionWrapper(v8::Local<v8::Context>& ctx, const v8::Handle<v8::Value>& val);
  virtual ~jsFunctionWrapper();

  virtual unsigned long AddRef();
  virtual unsigned long Release();
  virtual unsigned long getRefCount() const {
    return mRefCount;
  }
  virtual size_t hash() {
    return mHash;
  }

  virtual void setHash(size_t hash) {
    UNUSED_PARAM(hash);
  }
 
  void signal(rtValue const& returnValue);

private:
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result);

  rtValue wait();
  void setupSynchronousWait();

  class FunctionLookup : public jsIFunctionLookup
  {
  public:
    FunctionLookup(jsFunctionWrapper* parent) : mParent(parent) { }
    virtual v8::Local<v8::Function> lookup(v8::Local<v8::Context>& ctx);
  private:
    rtFunctionRef mParent;
  };

  friend class FunctionLookup;

private:
  unsigned long mRefCount;
  v8::Persistent<v8::Function> mFunction;
  v8::Persistent<v8::Context> mContext;
  v8::Isolate* mIsolate;
  std::vector<rtValue> mArgs;

  bool mComplete;
  bool mTeardownThreadingPrimitives;

#ifdef USE_STD_THREADS
  std::mutex mMutex;
  std::condition_variable mCond;
#else
  pthread_mutex_t mMutex;
  pthread_cond_t mCond;
#endif
  size_t mHash;
  rtValue mReturnValue;
};

class rtResolverFunction : public rtAbstractFunction
{
public:
  enum Disposition
  {
    DispositionResolve,
    DispositionReject
  };

  rtResolverFunction(Disposition d, v8::Local<v8::Context>& ctx, v8::Local<v8::Promise::Resolver>& resolver);
  virtual ~rtResolverFunction();
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result);
  virtual size_t hash()
  {
    return -1;
  }

  virtual void setHash(size_t hash) {
    UNUSED_PARAM(hash);
  }


private:
  struct AsyncContext
  {
    rtFunctionRef resolverFunc;
    std::vector<rtValue> args;
  };

private:
  static void workCallback(uv_work_t* req);
  static void afterWorkCallback(uv_work_t* req, int status);

private:
  Disposition                         mDisposition;
  v8::Persistent<v8::Promise::Resolver>   mResolver;
  v8::Persistent<v8::Context>         mContext;
  v8::Isolate*                        mIsolate;
  uv_work_t                           mReq;
};

} // namespace

#endif
