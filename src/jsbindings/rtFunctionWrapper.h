#ifndef RT_FUNCTION_WRAPPER_H
#define RT_FUNCTION_WRAPPER_H

#include "rtWrapperUtils.h"
#include "jsCallback.h"

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
#ifdef ENABLE_NODE_V_6_9
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
    jsFunctionWrapper* mParent;
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

#endif
