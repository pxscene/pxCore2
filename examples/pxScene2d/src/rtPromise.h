// rtCore Copyright 2007-2015 John Robinson
// rtPromise.h

#ifndef _RT_PROMISE_H
#define _RT_PROMISE_H

#include <string>

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

  rtPromise() :  promise_id(promiseID++), promise_name(""), mState(PENDING), mObject(NULL)
  {
//    rtLogDebug("############# PROMISE >> CREATED   [ id: %d ]  \n", promise_id);
  }

  rtPromise(uint32_t id) : promise_id(id), promise_name("") , mState(PENDING), mObject(NULL)
  {
//    rtLogDebug("############# PROMISE >> CREATED   [ id: %d ]  \n", promise_id);
  }

  rtPromise(std::string name) : promise_id(promiseID++), promise_name(name), mState(PENDING), mObject(NULL)
  {
    rtLogDebug("############# PROMISE >> CREATED   [ id: %d  >> %s ]", promise_id, promise_name.c_str());
  }

  rtError then(rtFunctionRef resolve, rtFunctionRef reject, 
               rtObjectRef& newPromise)
  {
    if (mState == PENDING)
    {
      thenData data(resolve, reject, new rtPromise);

      mThenData.push_back(data);

      newPromise = data.mNextPromise;

      rtLogDebug("############# PROMISE >> THEN >> PENDING       [ id: %d  >> %s ]",
        promise_id, promise_name.c_str());
    }
    else if (mState == FULFILLED)
    {
      rtLogDebug("############# PROMISE >> THEN >> FULFILLED     [ id: %d  >> %s ]",
        promise_id, promise_name.c_str());

      if (resolve)
      {
        resolve.send(mObject);
        newPromise = new rtPromise;
        newPromise.send("resolve",mObject);
      }
    }
    else if (mState == REJECTED)
    {
      if (reject)
      {
        reject.send(mObject);
        newPromise = new rtPromise;
        newPromise.send("reject",mObject);
      }
    }
    else
    {
      rtLogError("Invalid rtPromise state");
    }

    return RT_OK;
  }

  rtError resolve(const rtValue& v)
  {
    if (mState != PENDING)
    {
      rtLogDebug("############# ERROR:  resolve() >> [%s] != PENDING   [ id: %d  >> %s ]",
        rtPromiseState2String(mState), promise_id,promise_name.c_str());

      return RT_FAIL;
    }

    mState = FULFILLED;
    mObject = v.toObject().getPtr();

    for (vector<thenData>::iterator it = mThenData.begin();
         it != mThenData.end(); ++it)
    {
      if (it->mResolve)
      {
        it->mResolve.send(mObject);
        it->mNextPromise.send("resolve",mObject);
      }
    }
    mThenData.clear();
    return RT_OK;
  }

  rtError reject(const rtValue& v)
  {
    if (mState != PENDING)
    {
      rtLogDebug("############# ERROR:  reject() >>[%s] != PENDING   [ id: %d  >> %s ]",
        rtPromiseState2String(mState), promise_id,promise_name.c_str());

      return RT_FAIL;
    }

    mState = REJECTED;
    rtObjectRef objRef = v.toObject();

    if (objRef.getPtr() != NULL)
    {
      mObject = objRef.getPtr();
    }
    if (mObject != NULL)
    {
      for (vector<thenData>::iterator it = mThenData.begin();
         it != mThenData.end(); ++it)
      {
        if (it->mReject)
        {
          it->mReject.send(mObject);
          it->mNextPromise.send("reject",mObject);
        }
      }
    }
    mThenData.clear();
    mObject = objRef.getPtr();
    return RT_OK;
  }

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
  vector<thenData> mThenData;
  rtIObject* mObject;
};

// uint32_t rtPromise::promiseID = 0;


#endif
