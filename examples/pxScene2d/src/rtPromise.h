// pxCore CopyRight 2007-2015 John Robinson
// rtPromise.h

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

  rtPromise():mState(PENDING), mObject(NULL) {}

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
      rtLogError("Invalid rtPromise state");
    return RT_OK;
  }

  rtError resolve(const rtValue& v)
  {
    if (mState != PENDING)
    {
      return RT_OK;
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
      return RT_OK;
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
      mThenData.clear();
    }
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
  
private:
  rtPromiseState mState;
  vector<thenData> mThenData;
  rtIObject* mObject;
};
