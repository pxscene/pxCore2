#ifndef RT_FUNCTION_WRAPPER_H
#define RT_FUNCTION_WRAPPER_H

#include "node_headers.h"

#include "rtWrapperUtils.h"

extern "C" {
#include "duv.h"
}

class rtFunctionWrapper : public rtWrapper<rtFunctionRef, rtFunctionWrapper>
{
public:
  rtFunctionWrapper(const rtFunctionRef& ref);
  virtual ~rtFunctionWrapper();

public:
  static void createFromFunctionReference(duk_context *ctx, const rtFunctionRef& func);
};

class jsFunctionWrapper : public rtIFunction
{
public:
  jsFunctionWrapper(duk_context *ctx, const std::string &funcName) : mRefCount(0), mDukCtx(ctx), mDukFuncName(funcName) {}
  virtual ~jsFunctionWrapper();

  virtual unsigned long AddRef();
  virtual unsigned long Release();
  virtual unsigned long getRefCount() const {
    return mRefCount;
  }
#if 0
  void signal(rtValue const& returnValue);
#endif

private:
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result);

#if 0
  rtValue wait();
  void setupSynchronousWait();
#endif

private:
  unsigned long mRefCount;
  std::vector<rtValue> mArgs;

  duk_context *mDukCtx;
  std::string  mDukFuncName;

  bool mComplete;
  bool mTeardownThreadingPrimitives;

#if 0
#ifdef USE_STD_THREADS
  std::mutex mMutex;
  std::condition_variable mCond;
#else
  pthread_mutex_t mMutex;
  pthread_cond_t mCond;
#endif
#endif

  rtValue mReturnValue;
};

#endif
