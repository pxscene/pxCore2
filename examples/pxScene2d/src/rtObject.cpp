// rtCore CopyRight 2005-2015 John Robinson
// rtObject.cpp

#include "rtObject.h"


rtError rtObjectBase::Send(const char* messageName, int numArgs, 
			   const rtValue* args, rtValue& result) 
{
  rtError e = RT_ERROR;
  rtFunctionRef f;
  e = get<rtFunctionRef>(messageName, f);
  if (e == RT_OK) {
    e = f->Send(numArgs, args, &result);
  }
  return e;
}

rtError rtObjectBase::send(const char* messageName) {
  rtValue discardResult;
  return Send(messageName, 0, 0, discardResult);
}

rtError rtObjectBase::send(const char* messageName, const rtValue& arg1) {
  rtValue discardResult;
  rtValue args[1] = {arg1};
  return Send(messageName, 1, args, discardResult);
}

rtError rtObjectBase::send(const char* messageName, const rtValue& arg1, 
			   const rtValue& arg2) {
  rtValue discardResult;
  rtValue args[2] = {arg1, arg2};
  return Send(messageName, 2, args, discardResult);
}

rtError rtObjectBase::send(const char* messageName, const rtValue& arg1, 
			   const rtValue& arg2, const rtValue& arg3) {
  rtValue discardResult;
  rtValue args[3] = {arg1, arg2, arg3};
  return Send(messageName, 3, args, discardResult);
}

rtError rtObjectBase::send(const char* messageName, const rtValue& arg1, 
			   const rtValue& arg2, const rtValue& arg3, 
			   const rtValue& arg4)
{
  rtValue discardResult;
  rtValue args[4] = {arg1, arg2, arg3, arg4};
  return Send(messageName, 4, args, discardResult);
}

rtError rtObjectBase::send(const char* messageName, const rtValue& arg1, 
			   const rtValue& arg2, const rtValue& arg3, 
			   const rtValue& arg4, const rtValue& arg5)
{
  rtValue discardResult;
  rtValue args[5] = {arg1, arg2, arg3, arg4, arg5};
  return Send(messageName, 5, args, discardResult);
}

rtError rtFunctionBase::send()
{
  rtValue discardResult;
  return Send(0, 0, discardResult);
}

rtError rtFunctionBase::send(const rtValue& arg1)
{
  rtValue discardResult;
  rtValue args[1] = {arg1};
  return Send(1, args, discardResult);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2)
{
  rtValue discardResult;
  rtValue args[2] = {arg1, arg2};
  return Send(2, args, discardResult);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2, 
			     const rtValue& arg3)
{
  rtValue discardResult;
  rtValue args[3] = {arg1, arg2, arg3};
  return Send(3, args, discardResult);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2, 
			     const rtValue& arg3, const rtValue& arg4)
{
  rtValue discardResult;
  rtValue args[4] = {arg1, arg2, arg3, arg4};
  return Send(4, args, discardResult);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2, 
			     const rtValue& arg3, const rtValue& arg4,
			     const rtValue& arg5)
{
  rtValue discardResult;
  rtValue args[5] = {arg1, arg2, arg3, arg4, arg5};
  return Send(5, args, discardResult);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2, 
			     const rtValue& arg3, const rtValue& arg4,
			     const rtValue& arg5, const rtValue& arg6)
{
  rtValue discardResult;
  rtValue args[6] = {arg1, arg2, arg3, arg4, arg5, arg6};
  return Send(6, args, discardResult);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2, 
			     const rtValue& arg3, const rtValue& arg4,
			     const rtValue& arg5, const rtValue& arg6,
			     const rtValue& arg7)
{
  rtValue discardResult;
  rtValue args[7] = {arg1, arg2, arg3, arg4, arg5, arg6, arg7};
  return Send(7, args, discardResult);
}


rtError rtObjectRef::Get(const char* name, rtValue* value) {
  return (*this)->Get(name, value);
}
 
rtError rtObjectRef::Set(const char* name, const rtValue* value) {
  return (*this)->Set(name, value);
}

rtError rtFunctionRef::Send(int numArgs, const rtValue* args, rtValue* result) {
  return (*this)->Send(numArgs, args, result);
}

rtError rtObjectFunction::Send(int numArgs, const rtValue* args, rtValue* result){  
  return (*mObject.*mThunk)(numArgs, args, *result);
}

rtObject::~rtObject() {}

rtError rtObject::description(rtString& d) {
    d = getMap()->className;
    return RT_OK;
}

#if 0
rtError rtAlloc(const char* objectName, rtObjectRef& object) {
  return RT_OK;
}
#endif

rtDefineObjectPtr(rtObject, NULL);

rtDefineMethod(rtObject, description);
rtDefineMethod(rtObject, init);


