#ifndef RT_FUNCTION_WRAPPER_H
#define RT_FUNCTION_WRAPPER_H

#include "rtWrapperUtils.h"


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

#endif
