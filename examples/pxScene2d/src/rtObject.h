// rtCore CopyRight 2005-2015 John Robinson
// rtObject.h

#ifndef RT_OBJECT_H
#define RT_OBJECT_H

#include "rtLog.h"
#include "rtAtomic.h"
#include "rtError.h"
#include "rtString.h"
#include "rtValue.h"
#include "rtObjectMacros.h"
#include "rtRefT.h"

#include <string.h>
#include <vector>
using namespace std;

// rtIObject and rtIFunction are designed to be an
// Abstract Binary Interface(ABI)
// suitable for providing stable inter-module contracts

class rtIObject {
 public:
  virtual ~rtIObject() { }
  virtual unsigned long AddRef() = 0;
  virtual unsigned long Release() = 0;
  virtual rtError Get(const char* name, rtValue* value) = 0;
  virtual rtError Get(uint32_t i, rtValue* value) = 0;
  virtual rtError Set(const char* name, const rtValue* value) = 0;
  virtual rtError Set(uint32_t i, const rtValue* value) = 0;
};

class rtIFunction {
 public:
  virtual ~rtIFunction() { }
  virtual unsigned long AddRef() = 0;
  virtual unsigned long Release() = 0;
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result) = 0;
};

class rtObjectRef;

// Mix-in providing convenience methods for rtIObject(s)
class rtObjectBase
{
public:

  template<typename T>
    rtError get(const char* name, T& value);  
  template<typename T>
    T get(const char* name);
  template<typename T>
    rtError get(uint32_t i, T& value);  
  template<typename T>
    T get(uint32_t i);

  void set(rtObjectRef o);

  finline rtError set(const char* name, const rtValue& value) {
    return Set(name, &value);
  }

  finline rtError set(uint32_t i, const rtValue& value) {
    return Set(i, &value);
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
  virtual rtError Send(const char* messageName, int numArgs, const rtValue* args);
  rtError SendReturns(const char* messageName, int numArgs, 
                      const rtValue* args,
                      rtValue& result); 

 private:
  virtual rtError Get(const char* name, rtValue* value) = 0;
  virtual rtError Get(uint32_t i, rtValue* value) = 0;
  virtual rtError Set(const char* name, const rtValue* value) = 0;
  virtual rtError Set(uint32_t i, const rtValue* value) = 0;
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

  finline rtError Send(int numArgs, const rtValue* args) 
  {
    return Send(numArgs, args, NULL);
  }

  finline rtError SendReturns(int numArgs, const rtValue* args, 
                              rtValue& result) 
  {
    return Send(numArgs, args, &result);
  }

 private:
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result) = 0;
};

class rtObjectRef: public rtRefT<rtIObject>, public rtObjectBase
{
 public:
  rtObjectRef() {}
  rtObjectRef(const rtIObject* o) { asn(o); }

  // operator= is not inherited
  rtObjectRef& operator=(rtIObject* o) { asn(o); return *this; }

 private:
  virtual rtError Get(const char* name, rtValue* value);
  virtual rtError Get(uint32_t i, rtValue* value);
  virtual rtError Set(const char* name, const rtValue* value);
  virtual rtError Set(uint32_t i, const rtValue* value);
};

class rtFunctionRef: public rtRefT<rtIFunction>, public rtFunctionBase
{
 public:
  rtFunctionRef() {}
  rtFunctionRef(const rtIFunction* f) { asn(f); }

  // operator= is not inherited
  rtFunctionRef& operator=(rtIFunction* f) { asn(f); return *this; }

 private:
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result);
};

class rtObjectFunction: public rtIFunction, public rtFunctionBase {
public:
  rtObjectFunction(rtObject* o, rtMethodThunk t): mRefCount(0) 
  {
    mObject = o;
    mThunk = t;
  }
  virtual ~rtObjectFunction() {}
  
  virtual unsigned long AddRef() { return rtAtomicInc(&mRefCount); }
  virtual unsigned long Release() 
  {
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

class rtObject: public rtIObject, public rtObjectBase  
{
public:
  rtDeclareObjectBase();
  
  // Define Dynamic API
  rtMethodNoArgAndNoReturn("init", init);
  // TODO change this into a readonly property?
  rtMethodNoArgAndReturn("description", description, rtString);
  rtReadOnlyProperty(allKeys, allKeys, rtObjectRef);
  
  rtObject(): mInitialized(false), mRefCount(0) { }
  virtual ~rtObject();
  
  virtual unsigned long /*__stdcall*/ AddRef();
  virtual unsigned long /*__stdcall*/ Release();

#if 1

  // hook for doing "post construction" activities
  virtual void onInit() {}
  
  rtError init();

  rtError description(rtString& d) const;
  rtError allKeys(rtObjectRef& v) const;

  virtual rtError Get(uint32_t /*i*/, rtValue* /*value*/);
  virtual rtError Get(const char* name, rtValue* value);
  virtual rtError Set(uint32_t /*i*/, const rtValue* /*value*/);
  virtual rtError Set(const char* name, const rtValue* value);

protected:
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
rtError rtObjectBase::get(const char* name, T& value) 
{
  rtValue v;
  rtError e = Get(name, &v);
  if (e == RT_OK) 
    value = v.convert<T>();
  return e;
}

template<typename T>
T rtObjectBase::get(const char* name) 
{
  rtValue v;
  Get(name, &v);
  return v.convert<T>();
}

template<typename T>
T rtObjectBase::get(uint32_t i) 
{
  rtValue v;
  Get(i, &v);
  return v.convert<T>();
}

template<typename T>
rtError rtObjectBase::get(uint32_t i, T& value) 
{
  rtValue v;
  rtError e = Get(i, &v);
  if (e == RT_OK) 
    value = v.convert<T>();
  return e;
}

template <typename T> 
rtError rtObjectBase::sendReturns(const char* messageName, T& result) 
{
  rtValue resultValue;
  rtError e = SendReturns(messageName, 0, 0, resultValue);
  if (e == RT_OK) 
    result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtObjectBase::sendReturns(const char* messageName, const rtValue& arg1, 
				  T& result) 
{
  rtValue args[1] = {arg1};
  rtValue resultValue;
  rtError e = SendReturns(messageName, 1, args, resultValue);
  if (e == RT_OK) 
    result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtObjectBase::sendReturns(const char* messageName, const rtValue& arg1, 
				  const rtValue& arg2, T& result) 
{
  rtValue args[2] = {arg1, arg2};
  rtValue resultValue;
  rtError e = SendReturns(messageName, 2, args, resultValue);
  if (e == RT_OK) 
    result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtObjectBase::sendReturns(const char* messageName, const rtValue& arg1, 
				  const rtValue& arg2, const rtValue& arg3, 
				  T& result)
{
  rtValue args[3] = {arg1, arg2, arg3};
  rtValue resultValue;
  rtError e = SendReturns(messageName, 3, args, resultValue);
  if (e == RT_OK) 
    result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtObjectBase::sendReturns(const char* messageName, const rtValue& arg1, 
				  const rtValue& arg2, const rtValue& arg3, 
				  const rtValue& arg4, T& result)
{
  rtValue args[] = {arg1, arg2, arg3, arg4};
  rtValue resultValue;
  rtError e = SendReturns(messageName, 4, args, resultValue);
  if (e == RT_OK) 
    result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtFunctionBase::sendReturns(T& result)
{
  rtValue resultValue;
  rtError e = SendReturns(0, 0, resultValue);
  if (e == RT_OK) 
    result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtFunctionBase::sendReturns(const rtValue& arg1, T& result)
{
  rtValue args[1] = {arg1};
  rtValue resultValue;
  rtError e = SendReturns(1, args, resultValue);
  if (e == RT_OK) 
    result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtFunctionBase::sendReturns(const rtValue& arg1, const rtValue& arg2, 
				    T& result)
{
  rtValue args[2] = {arg1, arg2};
  rtValue resultValue;
  rtError e = SendReturns(2, args, resultValue);
  if (e == RT_OK) 
    result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtFunctionBase::sendReturns(const rtValue& arg1, const rtValue& arg2, 
				    const rtValue& arg3, T& result)
{
  rtValue args[3] = {arg1, arg2, arg3};
  rtValue resultValue;
  rtError e = SendReturns(3, args, resultValue);
  if (e == RT_OK) 
    result = resultValue.convert<T>();
  return e;
}

template <typename T> 
rtError rtFunctionBase::sendReturns(const rtValue& arg1, const rtValue& arg2, 
				    const rtValue& arg3, const rtValue& arg4, 
				    T& result)
{
  rtValue args[] = {arg1, arg2, arg3, arg4};
  rtValue resultValue;
  rtError e = SendReturns(4, args, resultValue);
  if (e == RT_OK) 
    result = resultValue.convert<T>();
  return e;
}


typedef rtError (*rtFunctionCB)(int numArgs, const rtValue* args, rtValue* result, void* context);

class rtFunctionCallback: public rtIFunction 
{
public:
  rtFunctionCallback(rtFunctionCB cb, void* context = NULL) 
  {
    mCB = cb;
    mContext = context;
    mRefCount = 0;
  }
  
  virtual ~rtFunctionCallback() {}

  virtual unsigned long AddRef() 
  {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() 
  {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  virtual unsigned long getRefCount() const 
  {
    return mRefCount;
  }

  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result)
  {
    return mCB(numArgs, args, result, mContext);
  }

private:  
  rtFunctionCB mCB;
  void* mContext;
  rtAtomic mRefCount;
};

struct _rtEmitEntry 
{
  rtString n;
  rtFunctionRef f;
  bool isProp;
};

class rtEmit: public rtIFunction 
{

public:
  rtEmit(): mRefCount(0) {}
  virtual ~rtEmit() {}

  virtual unsigned long AddRef();
  virtual unsigned long Release();

  rtError setListener(const char* eventName, rtIFunction* f);
  rtError addListener(const char* eventName, rtIFunction* f);
  rtError delListener(const char* eventName, rtIFunction* f);

public:
  virtual rtError Send(int numArgs,const rtValue* args,rtValue* result);
    
  vector<_rtEmitEntry> mEntries;
  rtAtomic mRefCount;
};

class rtEmitRef: public rtRefT<rtEmit>, public rtFunctionBase
{
public:
  rtEmitRef() {}
  rtEmitRef(rtEmit* e) { asn(e); }

  // operator= is not inherited
  rtEmitRef& operator=(rtEmit* e) { asn(e); return *this; }

private:
  virtual rtError Send(int numArgs,const rtValue* args,rtValue* result);
};

class rtArrayObject: public rtObject 
{
public:
  rtArrayObject() {}
  
  void empty();
  void pushBack(rtValue v);

  virtual rtError Get(const char* name, rtValue* value);
  virtual rtError Get(uint32_t i, rtValue* value);
  virtual rtError Set(const char* /*name*/, const rtValue* /*value*/);
  virtual rtError Set(uint32_t i, const rtValue* value);

private:
  vector<rtValue> mElements;
};

struct rtNamedValue
{
  rtString n;
  rtValue v;
};

class rtMapObject: public rtObject 
{
public:

  vector<rtNamedValue>::iterator find(const char* name);

  virtual rtError Get(const char* name, rtValue* value);
  virtual rtError Set(const char* name, const rtValue* value);
  virtual rtError Get(uint32_t /*i*/, rtValue* /*value*/);
  virtual rtError Set(uint32_t /*i*/, const rtValue* /*value*/);

private:
  vector<rtNamedValue> mProps;
};

#endif
#endif

