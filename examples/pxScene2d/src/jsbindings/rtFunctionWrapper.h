#ifndef RT_FUNCTION_WRAPPER_H
#define RT_FUNCTION_WRAPPER_H

#include "rtWrapperUtils.h"
#include "jsCallback.h"


using namespace v8;

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
private:
  unsigned long mRefCount;
};

class rtFunctionWrapper : public rtWrapper<rtFunctionRef, rtFunctionWrapper>
{
public:
  rtFunctionWrapper(const rtFunctionRef& ref);
  virtual ~rtFunctionWrapper();

public:
  static void exportPrototype(v8::Isolate* isolate, Handle<Object> exports);

  static v8::Handle<v8::Object> createFromFunctionReference(v8::Isolate* isolate, const rtFunctionRef& func);

private:
  static void create(const FunctionCallbackInfo<Value>& args);
  static void call(const FunctionCallbackInfo<Value>& args);
};

class jsFunctionWrapper : public rtIFunction
{
public:
  jsFunctionWrapper(v8::Isolate* isolate, const Handle<Value>& val);
  ~jsFunctionWrapper();

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
    virtual Local<Function> lookup();
  private:
    jsFunctionWrapper* mParent;
  };

  friend class FunctionLookup;

private:
  unsigned long mRefCount;
  Persistent<Function> mFunction;
  Isolate* mIsolate;
  vector<rtValue> mArgs;

  bool mComplete;
  bool mTeardownThreadingPrimitives;
  pthread_mutex_t mMutex;
  pthread_cond_t mCond;
  rtValue mReturnValue;
};

#endif
