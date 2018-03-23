#include "optimus_client.h"

#include "rtRemote.h"

rtObjectRef gSceneContainer;
rtObjectRef gTestObj;
rtMutex rtRemoteOptimusMutex;
int pendingRemoteItems = 0;

namespace OptimusClient
{

  void rtRemoteOptimusCallback(void*)
  {
    rtLogDebug("inside rtRemoteCallback");
    rtRemoteOptimusMutex.lock();
    pendingRemoteItems++;
    rtRemoteOptimusMutex.unlock();
  }

  rtError pumpRemoteObjectQueue()
  {
    int numberOfItems = 0;
    rtRemoteOptimusMutex.lock();
    numberOfItems = pendingRemoteItems;
    pendingRemoteItems = 0;
    rtRemoteOptimusMutex.unlock();


    
    for (int i = 0; i < numberOfItems; i++)
    {
      rtError err = rtRemoteProcessSingleItem();
    }
    
    return RT_OK;
  }


  rtError registerApi(rtObjectRef object)
  {
    rtLogInfo("rtOptimus register API called");
    rtError e = RT_OK;
    rtRemoteEnvironment* env = rtEnvironmentGetGlobal();

    rtRemoteRegisterQueueReadyHandler( rtEnvironmentGetGlobal(), rtRemoteOptimusCallback, nullptr );

    e = rtRemoteInit(env);
    if (e != RT_OK)
    {
      rtLogError("rtRemoteInit:%s", rtStrError(e));
      return e;
    }

    const char* objectName = getenv("PX_WAYLAND_CLIENT_REMOTE_OBJECT_NAME");
    if (objectName == NULL)
    {
      rtLogWarn("unable to initialze rtOptimus due to invalid object name");
      return RT_FAIL;
    }
    e = rtRemoteRegisterObject(env, objectName, object);

    return e;
  }

  rtError getService(rtString name, rtObjectRef &returnObject)
  {
    fflush(stdout);
    if (gSceneContainer.getPtr())
    {
      rtLogInfo("scene container set, attempting to get service");
      gSceneContainer.sendReturns<rtObjectRef>("getService", name, returnObject);
      rtLogInfo("getService success");
      return RT_OK;
    }
    rtLogWarn("scene container not set");
    return RT_FAIL;
  }

  rtOptimus::rtOptimus() : mEmit(), mRemoteObject(), mTestObj()
  {
    mEmit = new rtEmit();
  }

  rtOptimus::~rtOptimus()
  {
    mEmit->clearListeners();
    shutdown();
  }

  rtError rtOptimus::shutdown()
  {
    return rtRemoteShutdown();
  }

  rtError rtOptimus::addListener(rtString eventName, const rtFunctionRef &f)
  {
    rtError rc = mEmit->addListener(eventName, f);

    return rc;
  }

  rtError rtOptimus::delListener(rtString eventName, const rtFunctionRef &f)
  {
    return mEmit->delListener(eventName, f);
  }

  rtError rtOptimus::sceneContainer(rtValue& v) const
  {
    v = gSceneContainer;
    return RT_OK;
  }

  rtError rtOptimus::setSceneContainer(const rtValue& v)
  {
    gSceneContainer = v.toObject();
    if (gSceneContainer.getPtr() != NULL)
    {
      return RT_OK;
    }
    rtLogInfo("rtOptimus::setSceneContainer gSceneContainer is null");
    return RT_FAIL;
  }

  rtDefineObject(rtOptimus, rtObject);

  rtDefineProperty(rtOptimus, sceneContainer);
  rtDefineMethod(rtOptimus, addListener);
  rtDefineMethod(rtOptimus, delListener);

}
