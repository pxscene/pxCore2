/*

 pxCore Copyright 2005-2017 John Robinson

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

// rtPromise.h

#ifndef _RT_PROMISE_H
#define _RT_PROMISE_H

// TODO Eliminate std::string
#include <string>
#include "jsbindings/rtWrapperUtils.h"

enum rtPromiseState {PENDING,FULFILLED,REJECTED};

struct thenData
{
  rtFunctionRef mResolve;
  rtFunctionRef mReject;
  rtObjectRef   mNextPromise;

  thenData(rtFunctionRef resolve,
           rtFunctionRef reject,
           rtObjectRef   next) :
      mResolve(resolve), mReject(reject), mNextPromise(next) {};

};

class rtPromise: public rtObject
{
  static uint32_t promiseID;

public:
  rtDeclareObject(rtPromise,rtObject);
  rtMethod2ArgAndReturn("then",then,rtFunctionRef,rtFunctionRef,rtObjectRef);
  rtMethod1ArgAndNoReturn("resolve",resolve,rtValue);
  rtMethod1ArgAndNoReturn("reject",reject,rtValue);
  rtMethod1ArgAndNoReturn("setResolve", setResolve, rtFunctionRef);
  rtMethod1ArgAndNoReturn("setReject", setReject, rtFunctionRef);
  rtProperty(promiseId, getPromiseId, setPromiseId, rtString);
  rtProperty(promiseContext, getPromiseContext, setPromiseContext, voidPtr);
  rtProperty(val, val, setVal, rtValue);
  rtReadOnlyProperty(isResolved, isResolved, uint32_t);

  rtPromise() :  promise_id(promiseID++), promise_name(""), mState(PENDING), mObject(NULL), mPromiseContext(NULL), mIsResolved(0)
  {
//    rtLogDebug("############# PROMISE >> CREATED   [ id: %d ]  \n", promise_id);
  }

  rtPromise(uint32_t id) : promise_id(id), promise_name("") , mState(PENDING), mObject(NULL), mPromiseContext(NULL), mIsResolved(0)
  {
//    rtLogDebug("############# PROMISE >> CREATED   [ id: %d ]  \n", promise_id);
  }

  rtPromise(std::string name) : promise_id(promiseID++), promise_name(name), mState(PENDING), mPromiseContext(NULL), mIsResolved(0)
  {
    rtLogDebug("############# PROMISE >> CREATED   [ id: %d  >> %s ]", promise_id, promise_name.c_str());
  }

  virtual ~rtPromise()
  {
    if (!mPromiseId.isEmpty()) {
      rtDukDelGlobalIdent((duk_context *)mPromiseContext, mPromiseId.cString());
    }
  }

  rtError then(rtFunctionRef, rtFunctionRef, 
               rtObjectRef&)
  {
    // not implemented
    assert(0);

    //if (mState == PENDING)
    //{
    //  thenData data(resolve, reject, new rtPromise);

    //  mThenData.push_back(data);

    //  newPromise = data.mNextPromise;

    //  rtLogDebug("############# PROMISE >> THEN >> PENDING       [ id: %d  >> %s ]",
    //    promise_id, promise_name.c_str());
    //}
    //else if (mState == FULFILLED)
    //{
    //  rtLogDebug("############# PROMISE >> THEN >> FULFILLED     [ id: %d  >> %s ]",
    //    promise_id, promise_name.c_str());

    //  if (resolve)
    //  {
    //    resolve.send(mObject);
    //    newPromise = new rtPromise;
    //    newPromise.send("resolve",mObject);
    //  }
    //}
    //else if (mState == REJECTED)
    //{
    //  if (reject)
    //  {
    //    reject.send(mObject);
    //    newPromise = new rtPromise;
    //    newPromise.send("reject",mObject);
    //  }
    //}
    //else
    //{
    //  rtLogError("Invalid rtPromise state");
    //}

    return RT_OK;
  }

  
  rtError isResolved(uint32_t& v) const { v = mIsResolved; return RT_OK; }

  rtError val(rtValue& v) const { v = mVal; return RT_OK; }
  rtError setVal(rtValue v) { mVal = v; return RT_OK; }

  rtError getPromiseId(rtString& v) const { v = mPromiseId; return RT_OK; }
  rtError setPromiseId(rtString v) { mPromiseId = v; return RT_OK; }

  rtError getPromiseContext(voidPtr& v) const { v = mPromiseContext; return RT_OK; }
  rtError setPromiseContext(voidPtr v) { mPromiseContext = v; return RT_OK; }

  rtError resolve(const rtValue& v)
  {
    if (resolveCb.getPtr()) {
      resolveCb.send(v);
    }
    mVal = v;
    mIsResolved = 1;
    return RT_OK;
  }

  rtError reject(const rtValue& v)
  {
    if (rejectCb.getPtr()) {
      rejectCb.send(v);
    }
    mVal = v;
    mIsResolved = 2;
    return RT_OK;
  }

  rtError setResolve(rtFunctionRef resolve)
  {
    assert(!resolveCb);
    resolveCb = resolve;
    return RT_OK;
  }

  rtError setReject(rtFunctionRef reject)
  {
    assert(!rejectCb);
    rejectCb = reject;
    return RT_OK;
  }

  typedef rtError(rtObject::*rtGetPropertyThunk)(rtValue& result) const;
  typedef rtError(rtObject::*rtSetPropertyThunk)(const rtValue& value);

  bool status()
  {
    if( mState == PENDING)
      return false;
    else 
      return true;
  }
  
  const char* rtPromiseState2String(rtPromiseState s)
  {
    switch(s)
    {
      case PENDING:   return "PENDING";
      case FULFILLED: return "FULFILLED";
      case REJECTED:  return "REJECTED";
      default:        return "(unknown rtPromiseState)";
    }
  }

          int promise_id;
  std::string promise_name;

private:
  rtPromiseState mState;
  std::vector<thenData> mThenData;
  rtIObject* mObject;

  rtFunctionRef resolveCb;
  rtFunctionRef rejectCb;

  rtString mPromiseId;
  voidPtr  mPromiseContext;

  uint32_t mIsResolved;
  rtValue mVal;
};

// uint32_t rtPromise::promiseID = 0;


#endif
