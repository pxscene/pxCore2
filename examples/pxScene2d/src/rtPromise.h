// rtCore Copyright 2007-2015 John Robinson
// rtPromise.h

#ifndef _RT_PROMISE_H
#define _RT_PROMISE_H

enum rtPromiseState {PENDING,FULFILLED,REJECTED};

struct thenData
{
  rtFunctionRef mResolve;
  rtFunctionRef mReject;
  rtObjectRef mNextPromise;
};

class rtPromise: public rtObject
{
public:
  rtDeclareObject(rtPromise,rtObject);

  rtMethod2ArgAndReturn("then",then,rtFunctionRef,rtFunctionRef,rtObjectRef);

  rtMethod1ArgAndNoReturn("resolve",resolve,rtValue);
  rtMethod1ArgAndNoReturn("reject",reject,rtValue);

  rtPromise():mState(PENDING) {}

  rtError then(rtFunctionRef resolve, rtFunctionRef reject, 
               rtObjectRef& newPromise)
  {
    if (mState == PENDING)
    {
      thenData d;
      d.mResolve = resolve;
      d.mReject = reject;
      d.mNextPromise = new rtPromise;
      mThenData.push_back(d);

      newPromise = d.mNextPromise;
    }
    else if (mState == FULFILLED)
    {
      if (resolve)
      {
        resolve.send(mValue);
        newPromise = new rtPromise;
        newPromise.send("resolve",mValue);
      }
    }
    else if (mState == REJECTED)
    {
      if (reject)
      {
        reject.send(mValue);
        newPromise = new rtPromise;
        newPromise.send("reject",mValue);
      }
    }
    else
      rtLogError("Invalid rtPromise state");
    return RT_OK;
  }

  rtError resolve(const rtValue& v)
  {
    mState = FULFILLED;
    mValue = v;
    for (vector<thenData>::iterator it = mThenData.begin();
         it != mThenData.end(); ++it)
    {
      if (it->mResolve)
      {
        it->mResolve.send(mValue);
        it->mNextPromise.send("resolve",mValue);
      }
    }
    mThenData.clear();
    return RT_OK;
  }

  rtError reject(const rtValue& v)
  {
    mState = REJECTED;
    mValue = v;
    for (vector<thenData>::iterator it = mThenData.begin();
         it != mThenData.end(); ++it)
    {
      if (it->mReject)
      {
        it->mReject.send(mValue);
        it->mNextPromise.send("reject",mValue);
      }
    }
    mThenData.clear();
    return RT_OK;
  }

  bool status()
  {
    if( mState == PENDING)
      return false;
    else 
      return true;
  }
  
private:
  rtPromiseState mState;
  vector<thenData> mThenData;
  rtValue mValue;
};

#endif
