// rtCore CopyRight 2005-2015 John Robinson
// rtObject.cpp

#include "rtObject.h"

void rtObjectBase::set(rtObjectRef o)
{
  if (!o) 
    return;

  rtObjectRef keys = o.get<rtObjectRef>("allKeys");
  if (keys)
  {
    uint32_t len = keys.get<uint32_t>("length");
    for (uint32_t i = 0; i < len; i++)
    {
      rtString key = keys.get<rtString>(i);
      set(key, o.get<rtValue>(key));
    }
  }
}

rtError rtObjectBase::Send(const char* messageName, int numArgs, 
			   const rtValue* args) 
{
  rtError e = RT_ERROR;
  rtFunctionRef f;
  e = get<rtFunctionRef>(messageName, f);
  if (e == RT_OK) {
    e = f->Send(numArgs, args, NULL);
  }
  return e;
}

rtError rtObjectBase::SendReturns(const char* messageName, int numArgs, 
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
  return Send(messageName, 0, 0);
}

rtError rtObjectBase::send(const char* messageName, const rtValue& arg1) {
  rtValue args[1] = {arg1};
  return Send(messageName, 1, args);
}

rtError rtObjectBase::send(const char* messageName, const rtValue& arg1, 
			   const rtValue& arg2) {
  rtValue args[2] = {arg1, arg2};
  return Send(messageName, 2, args);
}

rtError rtObjectBase::send(const char* messageName, const rtValue& arg1, 
			   const rtValue& arg2, const rtValue& arg3) {
  rtValue args[3] = {arg1, arg2, arg3};
  return Send(messageName, 3, args);
}

rtError rtObjectBase::send(const char* messageName, const rtValue& arg1, 
			   const rtValue& arg2, const rtValue& arg3, 
			   const rtValue& arg4)
{
  rtValue args[4] = {arg1, arg2, arg3, arg4};
  return Send(messageName, 4, args);
}

rtError rtObjectBase::send(const char* messageName, const rtValue& arg1, 
			   const rtValue& arg2, const rtValue& arg3, 
			   const rtValue& arg4, const rtValue& arg5)
{
  rtValue args[5] = {arg1, arg2, arg3, arg4, arg5};
  return Send(messageName, 5, args);
}

rtError rtFunctionBase::send()
{
  return Send(0, 0, NULL);
}

rtError rtFunctionBase::send(const rtValue& arg1)
{
  rtValue args[1] = {arg1};
  return Send(1, args, NULL);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2)
{
  rtValue args[2] = {arg1, arg2};
  return Send(2, args);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2, 
			     const rtValue& arg3)
{
  rtValue args[3] = {arg1, arg2, arg3};
  return Send(3, args);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2, 
			     const rtValue& arg3, const rtValue& arg4)
{
  rtValue args[4] = {arg1, arg2, arg3, arg4};
  return Send(4, args);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2, 
			     const rtValue& arg3, const rtValue& arg4,
			     const rtValue& arg5)
{
  rtValue args[5] = {arg1, arg2, arg3, arg4, arg5};
  return Send(5, args);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2, 
			     const rtValue& arg3, const rtValue& arg4,
			     const rtValue& arg5, const rtValue& arg6)
{
  rtValue args[6] = {arg1, arg2, arg3, arg4, arg5, arg6};
  return Send(6, args);
}

rtError rtFunctionBase::send(const rtValue& arg1, const rtValue& arg2, 
			     const rtValue& arg3, const rtValue& arg4,
			     const rtValue& arg5, const rtValue& arg6,
			     const rtValue& arg7)
{
  rtValue args[7] = {arg1, arg2, arg3, arg4, arg5, arg6, arg7};
  return Send(7, args);
}


rtError rtObjectRef::Get(const char* name, rtValue* value) {
  return (*this)->Get(name, value);
}
 
rtError rtObjectRef::Set(const char* name, const rtValue* value) {
  return (*this)->Set(name, value);
}

rtError rtObjectRef::Get(uint32_t i, rtValue* value) {
  return (*this)->Get(i, value);
}
 
rtError rtObjectRef::Set(uint32_t i, const rtValue* value) {
  return (*this)->Set(i, value);
}

rtError rtFunctionRef::Send(int numArgs, const rtValue* args, rtValue* result) {
  return (*this)->Send(numArgs, args, result);
}

rtError rtObjectFunction::Send(int numArgs, const rtValue* args, rtValue* result){  
  rtLogDebug("rtObjectFunction::Send(%d,...)", numArgs);
  return (*mObject.*mThunk)(numArgs, args, *result);
}

rtObject::~rtObject() {}

rtError rtObject::description(rtString& d) {
    d = getMap()->className;
    return RT_OK;
}

rtError rtObject::allKeys(rtObjectRef& v) const
{
  rtRefT<rtArrayObject> keys = new rtArrayObject;

  {
    rtMethodMap* m = getMap();      
    while(m) 
    {
      rtPropertyEntry* e = m->getFirstProperty();
      while(e) 
      {
        keys->pushBack(e->mPropertyName);
        e = e->mNext;
      }
      m = m->parentsMap;
    }
  }
      
  {
    rtMethodMap* m = getMap();
    while(m)
	  {
	    rtMethodEntry* e = m->getFirstMethod();
      while(e)
      {
        keys->pushBack(e->mMethodName);
        e = e->mNext;
      }
      m = m->parentsMap;
    }
	}

  v = keys;
  return RT_OK;
}


#if 0
rtError rtAlloc(const char* objectName, rtObjectRef& object) {
  return RT_OK;
}
#endif

rtDefineObjectPtr(rtObject, NULL);
rtDefineMethod(rtObject, description);
rtDefineProperty(rtObject, allKeys);
rtDefineMethod(rtObject, init);


