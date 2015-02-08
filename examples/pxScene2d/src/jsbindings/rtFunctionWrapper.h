#ifndef RT_FUNCTION_WRAPPER_H
#define RT_FUNCTION_WRAPPER_H

#include "rtWrapperUtils.h"
#include "jsCallback.h"


using namespace v8;

class rtFunctionWrapper : public rtWrapper<rtFunctionRef, rtFunctionWrapper>
{
public:
  rtFunctionWrapper(const rtFunctionRef& ref);
  virtual ~rtFunctionWrapper();

public:
  static void exportPrototype(Handle<Object> exports);

  static v8::Handle<v8::Object> createFromFunctionReference(const rtFunctionRef& func);

private:
  static Handle<Value> create(const Arguments& args);
  static Handle<Value> call(const Arguments& args);
};

class jsFunctionWrapper : public rtIFunction
{
public:
  jsFunctionWrapper(const Handle<Value>& val);
  ~jsFunctionWrapper();

  virtual unsigned long AddRef();
  virtual unsigned long Release();
  virtual unsigned long getRefCount() const {
    return mRefCount;
  }

private:
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result);

  class FunctionLookup : public jsIFunctionLookup
  {
  public:
    FunctionLookup(jsFunctionWrapper* parent) : mParent(parent) { }
    virtual Persistent<Function> lookup();
  private:
    jsFunctionWrapper* mParent;
  };

  friend class FunctionLookup;

private:
  unsigned long mRefCount;
  Persistent<Function> mFunction;
  vector<rtValue> mArgs;
};

#endif
