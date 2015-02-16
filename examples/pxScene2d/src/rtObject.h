// rtCore CopyRight 2005-2015 John Robinson
// rtObject.h

#ifndef RT_OBJECT_H
#define RT_OBJECT_H

#include <string.h>

using namespace std;
#include <vector>

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
  virtual ~rtIObject() { }
  virtual unsigned long AddRef() = 0;
  virtual unsigned long Release() = 0;
#if 1
  // Only for debugging purposes please
  // Probably can get the same utility by AddRef followed by Release?
  virtual unsigned long getRefCount() const = 0;
#endif
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
#if 1
  // Only for debugging purposes please
  virtual unsigned long getRefCount() const = 0;
#endif
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result) = 0;
};

class rtObjectRef;

// Mix-in providing convenience methods for rtIObject(s)
class rtObjectBase
{
public:

  template<typename T>
    finline rtError get(const char* name, T& value);  
  template<typename T>
    finline T get(const char* name);
  template<typename T>
    finline rtError get(uint32_t i, T& value);  
  template<typename T>
    finline T get(uint32_t i);

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
  rtError Send(const char* messageName, int numArgs, const rtValue* args,
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
    return *this;
  }

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
  rtFunctionRef(rtIFunction* f) {
    asn(f);
  }

  // operator= is not inherited
  rtFunctionRef& operator=(rtIFunction* f) {
    asn(f);
    return *this;
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
  virtual ~rtObjectFunction() {}
  
  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  virtual unsigned long getRefCount() const {
    return mRefCount;
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
  
  // TODO prefix members with m
 rtObject(): mInitialized(false), mRefCount(0) { }
  
  virtual ~rtObject();
  
  virtual unsigned long /*__stdcall*/ AddRef() {
    return rtAtomicInc(&mRefCount);
  }

  virtual unsigned long getRefCount() const {
    return mRefCount;
  }
  
  virtual unsigned long /*__stdcall*/ Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

#if 1

  // hook for doing "post construction" activities
  virtual void onInit() {}

    rtError init()
    {
        mInitialized = true;
	onInit();
        return RT_OK;
    }

    rtError description(rtString& d);
    rtError allKeys(rtObjectRef& v) const;

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

    virtual rtError Get(uint32_t /*i*/, rtValue* /*value*/)
    {
      return RT_PROP_NOT_FOUND;
    }

    virtual rtError Get(const char* name, rtValue* value)
    {
      rtError hr = RT_PROP_NOT_FOUND;
      rtMethodMap* m = getMap();
      
      while(m) 
      {
        rtPropertyEntry* e = m->getFirstProperty();
        while(e) 
        {
          if (strcmp(name, e->mPropertyName) == 0) 
          {
            //    found = true;
            rtGetPropertyThunk t = e->mGetThunk;
            hr = (*this.*t)(*value);
            return hr;
          }
          e = e->mNext;
        }
        m = m->parentsMap;
      }
      rtLogDebug("key: %s not found", name);
      
      {
        rtLogDebug("Looking for function as property: %s", name);
	
        rtMethodMap* m;
        m = getMap();
	
        while(m)
        {
          rtMethodEntry* e = m->getFirstMethod();
          while(e)
          {
            if (strcmp(name, e->mMethodName) == 0)
            {
              rtLogDebug("found method: %s", name);
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

    virtual rtError Set(uint32_t /*i*/, const rtValue* /*value*/)
    {
      return RT_PROP_NOT_FOUND;
    }

    virtual rtError Set(const char* name, const rtValue* value) 
    {
      rtError hr = RT_PROP_NOT_FOUND;
      //      bool found = false;

      rtMethodMap* m;
      m = getMap();
      
      while(m) {
	rtPropertyEntry* e = m->getFirstProperty();
	while(e) {
	  if (strcmp(name, e->mPropertyName) == 0) {
	    //    found = true;
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
  {
    value = v.convert<T>();
  }
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
  {
    value = v.convert<T>();
  }
  return e;
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


struct _rtEmitEntry {
  rtString n;
  rtFunctionRef f;
  bool isProp;
};

typedef rtError (*rtFunctionCB)(int numArgs, const rtValue* args, rtValue* result, void* context);

class rtFunctionCallback: public rtIFunction {
public:
  rtFunctionCallback(rtFunctionCB cb, void* context = NULL) {
    mCB = cb;
    mContext = context;
    mRefCount = 0;
  }
  
  virtual ~rtFunctionCallback() {}

  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  virtual unsigned long getRefCount() const {
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

class rtEmit: public rtIFunction {

public:
rtEmit(): mRefCount(0) {}
  virtual ~rtEmit() {}

  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  virtual unsigned long getRefCount() const {
    return mRefCount;
  }

  rtError setListener(const char* eventName, rtIFunction* f)
  {
    for (vector<_rtEmitEntry>::iterator it = mEntries.begin(); 
         it != mEntries.end(); it++)
    {
      _rtEmitEntry& e = (*it);
      if (e.n == eventName && e.isProp)
      {
        mEntries.erase(it);
        // There can only be one
        break;
      }
    }
    if (f)
    {
      _rtEmitEntry e;
      e.n = eventName;
      e.f = f;
      e.isProp = true;
      mEntries.push_back(e);      
    }

    return RT_OK;
  }

  rtError addListener(const char* eventName, rtIFunction* f)
  {
    if (!f) return RT_ERROR;
    // Only allow unique entries
    bool found = false;
    for (vector<_rtEmitEntry>::iterator it = mEntries.begin(); it != mEntries.end(); it++)
    {
      _rtEmitEntry& e = (*it);
      if (e.n == eventName && e.f.getPtr() == f && !e.isProp)
      {
        found = true;
        break;
      }
    }
    if (!found)
    {
      _rtEmitEntry e;
      e.n = eventName;
      e.f = f;
      e.isProp = false;
      mEntries.push_back(e);
    }

    return RT_OK;
  }

  rtError delListener(const char* eventName, rtIFunction* f)
  {
    for (vector<_rtEmitEntry>::iterator it = mEntries.begin(); it != mEntries.end(); it++)
    {
      _rtEmitEntry& e = (*it);
      if (e.n == eventName && e.f.getPtr() == f && !e.isProp)
      {
        mEntries.erase(it);
        // There can only be one
        break;
      }
    }

    return RT_OK;
  }

public:
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result) 
  {
    if (numArgs > 0)
    {
      rtString eventName = args[0].toString();
      rtLogDebug("rtEmit::Send %s\n", eventName.cString());
      for(vector<_rtEmitEntry>::iterator it = mEntries.begin(); it != mEntries.end(); it++)
      {
        _rtEmitEntry& e = (*it);
        if (e.n == eventName)
        {
          // have to invoke all no opportunity to return errors
          e.f->Send(numArgs-1, args+1, result);
          // TODO log error 
        }
      }
    }
    return RT_OK;
  }
    
  vector<_rtEmitEntry> mEntries;
  rtAtomic mRefCount;
};

class rtEmitRef: public rtRefT<rtEmit>, public rtFunctionBase
{
public:
  rtEmitRef() {}
  rtEmitRef(rtEmit* e) 
  {
    asn(e);
  }

  // operator= is not inherited
  rtEmitRef& operator=(rtEmit* e) {
    asn(e);
    return *this;
  }

private:
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result) {
    return (*this)->Send(numArgs, args, result);
  }
};



class rtArrayObject: public rtObject {
public:
  rtArrayObject() {}

  void empty()
  {
    mElements.empty();
  }

  void pushBack(rtValue v)
  {
    mElements.push_back(v);
  }

  virtual rtError Get(const char* name, rtValue* value)
  {
    if (!value) return RT_FAIL;
    if (!strcmp(name, "length"))
    {
      value->setUInt32(mElements.size());
      return RT_OK;
    }
    else
      return RT_PROP_NOT_FOUND;
  }

  virtual rtError Get(uint32_t i, rtValue* value)
  {
    if (!value) return RT_FAIL;
    if (i < mElements.size())
    {
      *value = mElements[i];
      return RT_OK;
    }
    else
      return RT_PROP_NOT_FOUND;
  }

  virtual rtError Set(const char* /*name*/, const rtValue* /*value*/)
  {
    return RT_PROP_NOT_FOUND;
  }

  virtual rtError Set(uint32_t i, const rtValue* value)
  {
    if (!value) return RT_FAIL;
    if (i < mElements.size())
    {
      mElements[i] = *value;
      return RT_OK;
    }
    else
      return RT_PROP_NOT_FOUND;
  }

private:
  vector<rtValue> mElements;
};

struct rtNamedValue
{
  rtString n;
  rtValue v;
};

class rtMapObject: public rtObject {
 public:

  vector<rtNamedValue>::iterator find(const char* name)
  {
    vector<rtNamedValue>::iterator it = mProps.begin(); 
    while(it != mProps.end())
    {
      if (it->n == name)
	return it;
      it++;
    }
    return it;
  }

  virtual rtError Get(const char* name, rtValue* value)
  {
    if (!value) return RT_FAIL;

    vector<rtNamedValue>::iterator it = find(name);
    if (it != mProps.end())
    {
      *value = it->v;
      return RT_OK;
    }
    else if (!strcmp(name, "allKeys"))
    {
      rtRefT<rtArrayObject> keys = new rtArrayObject;
      vector<rtNamedValue>::iterator it = mProps.begin();
      while(it != mProps.end())
      {
	keys->pushBack(it->n);
	it++;
      }
      *value = keys;
      return RT_OK;
    }
    return RT_PROP_NOT_FOUND;
  }

  virtual rtError Set(const char* name, const rtValue* value)
  {
    if (!value) return RT_FAIL;

    vector<rtNamedValue>::iterator it = find(name);
    if (it != mProps.end())
    {
      it->v = *value;
      return RT_OK;
    }
    else
    {
      rtNamedValue v;
      v.n = name;
      v.v = *value;
      mProps.push_back(v);
      return RT_OK;
    }
    return RT_PROP_NOT_FOUND;
  }

  virtual rtError Get(uint32_t i, rtValue* value)
  {
    return RT_PROP_NOT_FOUND;
  }

  virtual rtError Set(uint32_t i, const rtValue* value)
  {
    return RT_PROP_NOT_FOUND;
  }

 private:
  vector<rtNamedValue> mProps;
};

#endif
#endif

