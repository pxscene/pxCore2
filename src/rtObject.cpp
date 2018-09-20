/*

 pxCore Copyright 2005-2018 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// rtObject.cpp

#include "rtObject.h"
#include <errno.h>

using namespace std;

// rtEmit
unsigned long rtEmit::AddRef() 
{
  return rtAtomicInc(&mRefCount);
}
  
unsigned long rtEmit::Release() {
  long l = rtAtomicDec(&mRefCount);
  if (l == 0) delete this;
  return l;
}

rtError rtEmit::setListener(const char* eventName, rtIFunction* f)
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
    e.markForDelete = false;
    e.fnHash = f->hash();
    mEntries.push_back(e);      
  }
  
  return RT_OK;
}

rtError rtEmit::addListener(const char* eventName, rtIFunction* f)
{
  if (!eventName || !f)
    return RT_ERROR;
  // Only allow unique entries
  bool found = false;
  for (vector<_rtEmitEntry>::iterator it = mEntries.begin(); 
       it != mEntries.end(); it++)
  {
    _rtEmitEntry& e = (*it);
    // mHash check for javscript events callback 
    // markForDelete check is added to handle scenario where same handler is deleted and added immediately in same handler
    if (e.n == eventName && ((e.f.getPtr() == f) || ((f->hash() != (size_t)-1) && (e.fnHash == f->hash()) && (false == e.markForDelete))) && !e.isProp)
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
    e.markForDelete = false;
    e.fnHash = f->hash();
    if (!mProcessingEvents)
    {
      mEntries.push_back(e);
    }
    else
    {
      //save pending events to add later after current event processing is done
      mPendingEntriesToAdd.push_back(e);
    }
  }
  
  return RT_OK;
}

rtError rtEmit::delListener(const char* eventName, rtIFunction* f)
{
  if (!eventName || !f)
    return RT_ERROR;

  for (vector<_rtEmitEntry>::iterator it = mEntries.begin(); 
       it != mEntries.end(); it++)
  {
    _rtEmitEntry& e = (*it);
    if (e.n == eventName && ((e.f.getPtr() == f) || (((size_t)-1 != e.fnHash) && (e.fnHash == f->hash()))) && !e.isProp)
    {
      // if no events is being processed currently, remove the event entries
      if (!mProcessingEvents)
      	mEntries.erase(it);
      else
        it->markForDelete = true;
      // There can only be one
      break;
    }
  }
  return RT_OK;
}

rtError rtEmit::Send(int numArgs, const rtValue* args, rtValue* result) 
{
  (void)result;
  if (numArgs > 0)
  {
    rtString eventName = args[0].toString();
    // check whether the js call need to be synchronous or not
    bool sync = true;
    if (numArgs > 1)
    {
      rtType type = args[1].getType();
      if (RT_boolType == type)
      {
        sync = args[1].toBool();
      }
    }
    rtLogDebug("rtEmit::Send %s", eventName.cString());

    vector<_rtEmitEntry>::iterator it = mEntries.begin();
    
    mProcessingEvents = true;
    while (it != mEntries.end())
    {
      _rtEmitEntry& e = (*it);
      if (e.n == eventName)
      {
        // Do this here to make interop synchronous
        rtError err;
        rtValue discard;
        // have to invoke all no opportunity to return errors
        // SYNC EVENTS
#ifndef DISABLE_SYNC_EVENTS
        // SYNC EVENTS ... enables stopPropagation() ...
        //
        // pass NULL as final argument for indication of asynchronous call
        if (sync)
        {
          err = e.f->Send(numArgs-1, args+1, &discard);
        }
        else
        {
          err = e.f->Send(numArgs-1, args+1, NULL);
        }
#else

#warning "  >>>>>>  No SYNC EVENTS... stopPropagation() will be broken !!"

        err = e.f->Send(numArgs-1, args+1, NULL);
#endif
        if (err != RT_OK)
          rtLogInfo("failed to send. %s", rtStrError(err));

        // EPIPE means it's disconnected
        if (err == rtErrorFromErrno(EPIPE) || err == RT_ERROR_STREAM_CLOSED)
        {
          rtLogInfo("removing entry from remote client");
          it = mEntries.erase(it);
        }
        else
        {
          ++it;
        }
      }
      else
      {
        ++it;
      }
    }
    mProcessingEvents = false;
    it = mEntries.begin();
    while (it != mEntries.end())
    {
      if (true == it->markForDelete)
      {
        it = mEntries.erase(it);
      }
      else
      {
        ++it;
      }
    }

    vector<_rtEmitEntry>::iterator pendingit = mPendingEntriesToAdd.begin();
    while (pendingit != mPendingEntriesToAdd.end())
    {
      _rtEmitEntry& src = (*pendingit);
      _rtEmitEntry dest;
      dest.n = src.n;
      dest.f = src.f;
      dest.isProp = src.isProp;
      dest.markForDelete = src.markForDelete;
      dest.fnHash = src.fnHash;

      mEntries.push_back(dest);
      ++pendingit;
    }
    mPendingEntriesToAdd.clear();
  }
  return RT_OK;
}
        
// rtEmitRef
rtError rtEmitRef::Send(int numArgs,const rtValue* args,rtValue* result) 
{
  return (*this)->Send(numArgs, args, result);
}

// rtArrayObject
void rtArrayObject::empty()
{
  mElements.clear();
}

void rtArrayObject::pushBack(rtValue v)
{
  mElements.push_back(v);
}

rtError rtArrayObject::Get(const char* name, rtValue* value) const
{
  if (!value) 
    return RT_FAIL;
  if (!strcmp(name, "length"))
  {
    value->setUInt32((uint32_t)mElements.size());
    return RT_OK;
  }
  else
    return RT_PROP_NOT_FOUND;
}

rtError rtArrayObject::Get(uint32_t i, rtValue* value) const
{
  if (!value) 
    return RT_FAIL;
  if (i < mElements.size())
  {
    *value = mElements[i];
    return RT_OK;
  }
  else
    return RT_PROP_NOT_FOUND;
}

rtError rtArrayObject::Set(const char* /*name*/,const rtValue* /*value*/)
{
  return RT_PROP_NOT_FOUND;
}

rtError rtArrayObject::Set(uint32_t i, const rtValue* value)
{
  if (!value) 
    return RT_ERROR_INVALID_ARG;

  // zero-based index [0] -> mElements.size() == 1
  if (i >= mElements.size())
    mElements.resize(i + 1);

  mElements[i] = *value;
  return RT_OK;
}

// rtMapObject
vector<rtNamedValue>::iterator rtMapObject::find(const char* name)
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

rtError rtMapObject::Get(const char* name, rtValue* value) const
{
  if (!value) 
    return RT_FAIL;

  rtMapObject* this_ = const_cast<rtMapObject*>(this);
  
  vector<rtNamedValue>::iterator it = this_->find(name);
  if (it != mProps.end())
  {
    *value = it->v;
    return RT_OK;
  }
  else if (!strcmp(name, "description"))
  {
    // TODO UGH... need to rework these meta properties
    // change decription to a property and probably put a prefix on it
    return rtObject::Get(name, value);
  }
  else if (!strcmp(name, "allKeys"))
  {
    rtRefT<rtArrayObject> keys = new rtArrayObject;
    vector<rtNamedValue>::const_iterator it = this_->mProps.begin();
    while(it != this_->mProps.end())
    {
      // exclude allKeys
      if (it->n != "allKeys")
        keys->pushBack(it->n);
      it++;
    }
    *value = keys;
    return RT_OK;
  }
  return RT_PROP_NOT_FOUND;
}

rtError rtMapObject::Set(const char* name, const rtValue* value)
{
  if (!value) 
    return RT_FAIL;
  
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

rtError rtMapObject::Get(uint32_t /*i*/, rtValue* /*value*/) const
{
  return RT_PROP_NOT_FOUND;
}

rtError rtMapObject::Set(uint32_t /*i*/, const rtValue* /*value*/)
{
  return RT_PROP_NOT_FOUND;
}


// rtObject
  
unsigned long /*__stdcall__ */ rtObject::AddRef() 
{
  return rtAtomicInc(&mRefCount);
}

unsigned long /*__stdcall*/ rtObject::Release() 
{
  long l = rtAtomicDec(&mRefCount);
  if (l == 0) 
    delete this;
  return l;
}

rtError rtObject::init()
{
  mInitialized = true;
	onInit();
  return RT_OK;
}

rtError rtObject::Get(uint32_t /*i*/, rtValue* /*value*/) const
{
  return RT_PROP_NOT_FOUND;
}

rtError rtObject::Get(const char* name, rtValue* value) const
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

rtError rtObject::Set(uint32_t /*i*/, const rtValue* /*value*/)
{
  return RT_PROP_NOT_FOUND;
}

rtError rtObject::Set(const char* name, const rtValue* value) 
{
  rtError hr = RT_PROP_NOT_FOUND;
  
  rtMethodMap* m;
  m = getMap();
  
  while(m) 
  {
    rtPropertyEntry* e = m->getFirstProperty();
    while(e) 
    {
      if (strcmp(name, e->mPropertyName) == 0) 
      {
        if (e->mSetThunk) 
        {
          rtSetPropertyThunk t = e->mSetThunk;
          hr = (*this.*t)(*value);
        }
        else
        {
          hr = RT_FAIL;
          rtLogError("setter for %s is missing thunk.", name);
        }
        return hr;
      }
      e = e->mNext;
    }
    
    m = m->parentsMap;
  }
  
  return hr;
}

// rtObjectBase
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
  if (e == RT_OK)
    e = f->Send(numArgs, args, NULL);
  return e;
}

rtError rtObjectBase::SendReturns(const char* messageName, int numArgs, 
			   const rtValue* args, rtValue& result) 
{
  rtError e = RT_ERROR;
  rtFunctionRef f;
  e = get<rtFunctionRef>(messageName, f);
  if (e == RT_OK)
    e = f->Send(numArgs, args, &result);
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


rtError rtObjectRef::Get(const char* name, rtValue* value) const
{
  return (*this)->Get(name, value);
}
 
rtError rtObjectRef::Get(uint32_t i, rtValue* value) const
{
  return (*this)->Get(i, value);
}
 
rtError rtObjectRef::Set(const char* name, const rtValue* value) 
{
  return (*this)->Set(name, value);
}

rtError rtObjectRef::Set(uint32_t i, const rtValue* value) 
{
  return (*this)->Set(i, value);
}

rtError rtFunctionRef::Send(int numArgs, const rtValue* args, rtValue* result) 
{
  return (*this)->Send(numArgs, args, result);
}

rtError rtObjectFunction::Send(int numArgs, const rtValue* args, rtValue* result)
{  
  rtLogDebug("rtObjectFunction::Send(%d,...)", numArgs);
  return (*mObject.*mThunk)(numArgs, args, *result);
}

rtObject::~rtObject() {}

rtError rtObject::description(rtString& d) const
{
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
        // exclude allKeys
        if (strcmp(e->mPropertyName,"allKeys"))
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
// TODO
rtError rtAlloc(const char* objectName, rtObjectRef& object) 
{
  return RT_OK;
}
#endif

rtDefineObjectPtr(rtObject, NULL);
rtDefineMethod(rtObject, description);
rtDefineProperty(rtObject, allKeys);
rtDefineMethod(rtObject, init);

rtDefineObject(rtArrayObject, rtObject);
rtDefineProperty(rtArrayObject, length);

rtDefineObject(rtMapObject, rtObject);

