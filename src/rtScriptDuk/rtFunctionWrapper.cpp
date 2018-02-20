#include "rtFunctionWrapper.h"
#include "rtWrapperUtils.h"

#include <vector>

rtFunctionWrapper::rtFunctionWrapper(const rtFunctionRef& ref)
  : rtWrapper(ref)
{
}

rtFunctionWrapper::~rtFunctionWrapper()
{
}

static duk_ret_t dukFunctionStub(duk_context *ctx)
{
  duk_push_current_function(ctx);
  bool res = duk_get_prop_string(ctx, -1, "\xff""\xff""data");
  assert(res);
  rtIFunction* func = (rtIFunction*)duk_require_pointer(ctx, -1);

  // [curfunc prop]
  duk_pop(ctx);
  // [curfunc]
  duk_pop(ctx);
  // []

  int numArgs = duk_get_top(ctx);

  assert(numArgs < 16);
  rtValue args[16];

  for (int i = 0; i < numArgs; ++i) {
    duk_dup(ctx, i);
    args[i] = duk2rt(ctx);
    duk_pop(ctx);
  }

  rtValue result;
  func->Send(numArgs, args, &result);
  if (result.getType() != 0) {
    rt2duk(ctx, result);
    return 1;
  }
  return 0;
}


void rtFunctionWrapper::createFromFunctionReference(duk_context *ctx, const rtFunctionRef& func)
{
  duk_idx_t objidx = duk_push_c_function(ctx, &dukFunctionStub, DUK_VARARGS);
  duk_push_pointer(ctx, (void *)func.getPtr());
  bool res = duk_put_prop_string(ctx, objidx, "\xff""\xff""data");
  assert(res);

  func->AddRef();

  // [func]
}

jsFunctionWrapper::~jsFunctionWrapper()
{
  if (!mDukFuncName.empty()) {
    rtDukDelGlobalIdent(mDukCtx, mDukFuncName);
  }
}

#if 0
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
#endif

unsigned long jsFunctionWrapper::AddRef()
{
  return rtAtomicInc(&mRefCount);
}

unsigned long jsFunctionWrapper::Release()
{
  unsigned long l = rtAtomicDec(&mRefCount);
  if (l == 0) {
    delete this;
  }
  return l;
}

rtError jsFunctionWrapper::Send(int numArgs, const rtValue* args, rtValue* result)
{
  duk_bool_t res = duk_get_global_string(mDukCtx, mDukFuncName.c_str());
  assert(res);

  for (int i = 0; i < numArgs; ++i) {
    rt2duk(mDukCtx, args[i]);
  }

  duk_int_t rt = duk_pcall(mDukCtx, numArgs);

  if (rt != 0) {
    duv_dump_error(mDukCtx, -1);
    duk_pop(mDukCtx);
    return 1;
  }

  if (result != NULL) {
    *result = duk2rt(mDukCtx);
  }

  duk_pop(mDukCtx);
  return 0;
}
