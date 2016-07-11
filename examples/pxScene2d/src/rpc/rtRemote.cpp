#include "rtRemote.h"
#include "rtRemoteClient.h"
#include "rtRemoteConfig.h"
#include "rtRemoteServer.h"

#include <rtLog.h>
#include <mutex>
#include <thread>

static rtRemoteServer* gServer = nullptr;
static std::mutex gMutex;
std::shared_ptr<rtRemoteStreamSelector> gStreamSelector;

rtError
rtRemoteInit()
{
  rtError e = RT_OK;
  rtRemoteConfig::getInstance(true);

  std::lock_guard<std::mutex> lock(gMutex);
  if (gServer == nullptr)
  {
    gServer = new rtRemoteServer();
    e = gServer->open();
    if (e != RT_OK)
      rtLogError("failed to open rtRemoteServer. %s", rtStrError(e));
  };

  if (gServer == nullptr)
  {
    rtLogError("rtRemoteServer is null");
    e = RT_FAIL;
  }

  return e;
}

extern rtError rtRemoteShutdownStreamSelector();

rtError
rtRemoteShutdown()
{
  rtError e = rtRemoteShutdownStreamSelector();
  if (e != RT_OK)
  {
    rtLogWarn("error shutting down stream selector. %s", rtStrError(e));
  }

  if (gServer)
  {
    delete gServer;
    gServer = nullptr;
  }

  return RT_OK;
}

rtError
rtRemoteRegisterObject(char const* id, rtObjectRef const& obj)
{
  if (gServer == nullptr)
    return RT_FAIL;

  if (id == nullptr)
    return RT_ERROR_INVALID_ARG;

  if (!obj)
    return RT_ERROR_INVALID_ARG;

  return gServer->registerObject(id, obj);
}

rtError
rtRemoteLocateObject(char const* id, rtObjectRef& obj, int timeout)
{
  if (gServer == nullptr)
  {
    rtLogError("rtRemoteInit not called");
    return RT_FAIL;
  }

  if (id == nullptr)
  {
    rtLogError("invalid id (null)");
    return RT_ERROR_INVALID_ARG;
  }

  return gServer->findObject(id, obj, timeout);
}
