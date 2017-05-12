#include "pxServiceManager.h"

rtRemoteEnvironment* pxServiceManager::mEnv = nullptr;

rtError pxServiceManager::findServiceManager(rtObjectRef &result)
{
  if (mEnv == nullptr)
  {
    mEnv = rtEnvironmentGetGlobal();
    rtError e = rtRemoteInit(mEnv);

    if (e != RT_OK)
    {
      rtLogInfo("rtRemoteInit e = %d", e);
      return e;
    }
  }

  rtError e = rtRemoteLocateObject(mEnv, SERVICE_MANAGER_OBJECT_NAME, result);

  if (e != RT_OK)
    rtLogInfo("rtRemoteLocateObject " SERVICE_MANAGER_OBJECT_NAME " e = %d\n", e);

  return e;
}

