// rtCore CopyRight 2005-2015 John Robinson
// rtObject.h

#ifndef RT_OBJECT_H
#define RT_OBJECT_H

#include <string.h>

#include "rtLog.h"
#include "rtAtomic.h"
#include "rtError.h"
#include "rtString.h"
#include "rtValue.h"
#include "rtObjectMacros.h"
#include "rtRefT.h"

// rtIObject and rtIFunction are designed to be an Abstract Binary Interface(ABI)
// suitable for providing a stable inter-module contract

class rtIObject {
 public:
  virtual unsigned long AddRef() = 0;
  virtual unsigned long Release() = 0;
  virtual rtError Get(const char* name, rtValue* value) = 0;
  virtual rtError Set(const char* name, const rtValue* value) = 0;
};

class rtIFunction {
 public:
  virtual unsigned long AddRef() = 0;
  virtual unsigned long Release() = 0;
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result) = 0;
};

// Mix-in providing convenience methods for rtIObject(s)
class rtObjectBase
{
public:

  template<typename T>
    finline rtError get(const char* name, T& value);  
  template<typename T>
    finline T get(const char* name);

  finline rtError set(const char* name, const rtValue& value) {
    return Set(name, &value);
  }

  // convenience methods
  rtError send(const char* messageName);
  rtError send(const char* messageName, const rtValue& arg1);
  rtError send(const char* messageName, const rtValue& arg1, 
	       const rtValue& arg2);
  rtError send(const char* messageName, const rtValue& arg1, 
	       const rtValue& arg2, const rtValue& arg3);
  rtError send(const char* messageName, const rtValue& arg1, 
	       const rtValue& arg2, const rtValue& arg3, 
	       const rtValue& arg4);
  rtError send(const char* messageName, const rtValue& arg1, 
	       const rtValue& arg2, const rtValue& arg3, 
	       const rtValue& arg4, const rtValue& arg5);
  rtError send(const char* messageName, const rtValue& arg1, 
	       const rtValue& arg2, const rtValue& arg3, 
	       const rtValue& arg4, const rtValue& arg5,
	       const rtValue& arg6);
  rtError send(const char* messageName, const rtValue& arg1, 
	       const rtValue& arg2, const rtValue& arg3, 
	       const rtValue& arg4, const rtValue& arg5,
	       const rtValue& arg6, const rtValue& arg7);
  rtError send(const char* messageName, const rtValue& arg1, 
	       const rtValue& arg2, const rtValue& arg3, 
	       const rtValue& arg4, const rtValue& arg5,
	       const rtValue& arg6, const rtValue& arg7,
	       const rtValue& arg8);
  rtError send(const char* messageName, const rtValue& arg1, 
	       const rtValue& arg2, const rtValue& arg3, 
	       const rtValue& arg4, const rtValue& arg5,
	       const rtValue& arg6, const rtValue& arg7,
	       const rtValue& arg8, const rtValue& arg9);

  // convenience methods with return type coercion
  template <typename T> 
    rtError sendReturns(const char* messageName, T& result);  
  template <typename T> 
    rtError sendReturns(const char* messageName, const rtValue& arg1, 
			T& result);
  template <typename T> 
    rtError sendReturns(const char* messageName, const rtValue& arg1, 
			const rtValue& arg2, T& result);
  template <typename T> 
    rtError sendReturns(const char* messageName, const rtValue& arg1, 
			const rtValue& arg2, const rtValue& arg3, 
			T& result);
  template <typename T> 
    rtError sendReturns(const char* messageName, const rtValue& arg1, 
			const rtValue& arg2, const rtValue& arg3, 
			const rtValue& arg4, T& result);

  // General case
  rtError Send(const char* messageName, int numArgs, const rtValue* args,
	       rtValue& result); 

 private:
  virtual rtError Get(const char* name, rtValue* value) = 0;
  virtual rtError Set(const char* name, const rtValue* value) = 0;
};

// Mix-in providing convenience methods for rtIFunction(s)
class rtFunctionBase
{
public:
  rtError send();
  rtError send(const rtValue& arg1);
  rtError send(const rtValue& arg1, const rtValue& arg2);
  rtError send(const rtValue& arg1, const rtValue& arg2, 
	       const rtValue& arg3);
  rtError send(const rtValue& arg1, const rtValue& arg2, 
	       const rtValue& arg3, const rtValue& arg4);
  rtError send(const rtValue& arg1, const rtValue& arg2, 
	       const rtValue& arg3, const rtValue& arg4,
	       const rtValue& arg5);
  rtError send(const rtValue& arg1, const rtValue& arg2, 
	       const rtValue& arg3, const rtValue& arg4,
	       const rtValue& arg5, const rtValue& arg6);
  rtError send(const rtValue& arg1, const rtValue& arg2, 
	       const rtValue& arg3, const rtValue& arg4,
	       const rtValue& arg5, const rtValue& arg6,
	       const rtValue& arg7);
  
  template <typename T> 
    rtError sendReturns(T& result);
  template <typename T> 
    rtError sendReturns(const rtValue& arg1, T& result);
  template <typename T> 
    rtError sendReturns(const rtValue& arg1, const rtValue& arg2, 
			T& result);
  template <typename T> 
    rtError sendReturns(const rtValue& arg1, const rtValue& arg2, 
			const rtValue& arg3, T& result);
  template <typename T> 
    rtError sendReturns(const rtValue& arg1, const rtValue& arg2, 
			const rtValue& arg3, const rtValue& arg4, 
			T& result);

  finline rtError Send(int numArgs, const rtValue* args, rtValue& result) {
    return Send(numArgs, args, &result);
  }

 private:
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result) = 0;
};

class rtObjectRef: public rtRefT<rtIObject>, public rtObjectBase
{
 public:
  rtObjectRef() {}
  rtObjectRef(rtIObject* o) {
    asn(o);
  }

  // operator= is not inherited
  rtObjectRef& operator=(rtIObject* o) {
    asn(o);
  }

 private:
  virtual rtError Get(const char* name, rtValue* value);
  virtual rtError Set(const char* name, const rtValue* value);
};

class rtFunctionRef: public rtRefT<rtIFunction>, public rtFunctionBase
{
 public:
  rtFunctionRef() {}
  rtFunctionRef(rtIFunction* f) {
    asn(f);
  }

  // operator= is not inherited
  rtFunctionRef& operator=(rtIFunction* f) {
    asn(f);
  }

 private:
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result);
};

class rtObjectFunction: public rtIFunction, public rtFunctionBase {
 public:
 rtObjectFunction(rtObject* o, rtMethodThunk t): mRefCount(0) {
    mObject = o;
    mThunk = t;
  }
  
  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

 private:
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result);

  rtRefT<rtObject> mObject;
  rtMethodThunk mThunk;
  unsigned long mRefCount;
};
#endif


class rtObject: public rtIObject, public rtObjectBase  
{
 public:
  rtDeclareObjectBase();
  
  // Define Dynamic API
  rtMethodNoArgAndNoReturn("init", init);
  rtMethodNoArgAndReturn("description", description, rtString);
  
  // TODO prefix members with m
 rtObject(): mInitialized(false), mRefCount(0) { }
  
  virtual ~rtObject();
  
  virtual unsigned long /*__stdcall*/ AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long /*__stdcall*/ Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

#if 1

    rtError init()
    {
        mInitialized = true;
        return RT_OK;
    }

    rtError description(rtString& d);

#if 0
    void dump()
    {
        rtMethodEntry* cur = head();
        while(cur)
        {
            rtLog("%S\n", cur->mMethodName);
            cur = cur->mNext;
        }
    }
#endif

    virtual rtError Get(const char* name, rtValue* value)
    {
      rtError hr = RT_PROP_NOT_FOUND;
      
      bool found = false;
      
      rtMethodMap* m;
      m = getMap();
      
      while(m) {
	rtPropertyEntry* e = m->getFirstProperty();
	while(e) {
	  if (strcmp(name, e->mPropertyName) == 0) {
	    found = true;
	    rtGetPropertyThunk t = e->mGetThunk;
	    hr = (*this.*t)(*value);
	    return hr;
	  }
	  e = e->mNext;
	}
	  
	m = m->parentsMap;
      }
      rtLog("key not found\n");
      
      {
	
	rtLog("Looking for function as property: %s\n", name);
	//if (!initialized) return RT_OBJECT_NOT_INITIALIZED;
	rtError hr = RT_PROP_NOT_FOUND;
	bool found = false;
	
	rtMethodMap* m;
	m = getMap();
	
	while(m)
	  {
	    rtMethodEntry* e = m->getFirstMethod();
            while(e)
	      {
                if (strcmp(name, e->mMethodName) == 0)
		  {
                    found = true;
		    rtLog("found method\n");
		    value->setFunction(new rtObjectFunction(this, e->mThunk));
		    hr = RT_OK;
                    return hr;
		  }
                e = e->mNext;
	      }
            
            m = m->parentsMap;
	    }
	}
        return hr;
    }

    virtual rtError Set(const char* name, const rtValue* value) 
    {
      rtError hr = RT_PROP_NOT_FOUND;
      bool found = false;
      
      rtMethodMap* m;
      m = getMap();
      
      while(m) {
	rtPropertyEntry* e = m->getFirstProperty();
	while(e) {
	  if (strcmp(name, e->mPropertyName) == 0) {
	    found = true;
	    if (e->mSetThunk) {
	      rtSetPropertyThunk t = e->mSetThunk;
	      hr = (*this.*t)(*value);
	    }
	    return hr;
	  }
	  e = e->mNext;
	}
            
	m = m->parentsMap;
      }

      return RT_OK;
    }

private:
    bool mInitialized;
    rtAtomic mRefCount;
};

#if 0
rtError rtAllocFromModule(const char* pathToDLL, const char* objectName, rtIObject** o);
rtError rtAllocFromModule(const char* pathToDLL, const char* objectName, rtObjectRef& object);
rtError rtAlloc(const char* objectName, rtObjectRef& object);
#endif


// TODO move out to another file

template<typename T>
finline rtError rtObjectBase::get(const char* name, T& value) 
{
  rtValue v;
  rtError e = Get(name, &v);
  if (e == RT_OK) {
    value = v.convert<T>();
  }
  return e;
}

template<typename T>
finline T rtObjectBase::get(const char* name) 
{
  rtValue v;
  Get(name, &v);
  return v.convert<T>();
}

template <typename T> 
rtError rtObjectBase::sendReturns(const char* messageName, T& result) 
{
  rtValue resultValue;
  rtError e = Send(messageName, 0, 0, resultValue);
  if (e == RT_OK) result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtObjectBase::sendReturns(const char* messageName, const rtValue& arg1, 
				  T& result) 
{
  rtValue args[1] = {arg1};
  rtValue resultValue;
  rtError e = Send(messageName, 1, args, resultValue);
  if (e == RT_OK) result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtObjectBase::sendReturns(const char* messageName, const rtValue& arg1, 
				  const rtValue& arg2, T& result) 
{
  rtValue args[2] = {arg1, arg2};
  rtValue resultValue;
  rtError e = Send(messageName, 2, args, resultValue);
  if (e == RT_OK) result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtObjectBase::sendReturns(const char* messageName, const rtValue& arg1, 
				  const rtValue& arg2, const rtValue& arg3, 
				  T& result)
{
  rtValue args[3] = {arg1, arg2, arg3};
  rtValue resultValue;
  rtError e = Send(messageName, 3, args, resultValue);
  if (e == RT_OK) result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtObjectBase::sendReturns(const char* messageName, const rtValue& arg1, 
				  const rtValue& arg2, const rtValue& arg3, 
				  const rtValue& arg4, T& result)
{
  rtValue args[3] = {arg1, arg2, arg3, arg4};
  rtValue resultValue;
  rtError e = Send(messageName, 4, args, resultValue);
  if (e == RT_OK) result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtFunctionBase::sendReturns(T& result)
{
  rtValue resultValue;
  rtError e = Send(0, 0, resultValue);
  if (e == RT_OK) result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtFunctionBase::sendReturns(const rtValue& arg1, T& result)
{
  rtValue args[1] = {arg1};
  rtValue resultValue;
  rtError e = Send(1, args, resultValue);
  if (e == RT_OK) resultValue.cvt(result);
  return e;
}

template <typename T> 
rtError rtFunctionBase::sendReturns(const rtValue& arg1, const rtValue& arg2, 
				    T& result)
{
  rtValue args[2] = {arg1, arg2};
  rtValue resultValue;
  rtError e = Send(2, args, resultValue);
  if (e == RT_OK) result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtFunctionBase::sendReturns(const rtValue& arg1, const rtValue& arg2, 
				    const rtValue& arg3, T& result)
{
  rtValue args[3] = {arg1, arg2, arg3};
  rtValue resultValue;
  rtError e = Send(3, args, resultValue);
  if (e == RT_OK) result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtFunctionBase::sendReturns(const rtValue& arg1, const rtValue& arg2, 
				    const rtValue& arg3, const rtValue& arg4, 
				    T& result)
{
  rtValue args[3] = {arg1, arg2, arg3, arg4};
  rtValue resultValue;
  rtError e = Send(4, args, resultValue);
  if (e == RT_OK) result = resultValue.convert<T>();
  return e;
}


#endif
