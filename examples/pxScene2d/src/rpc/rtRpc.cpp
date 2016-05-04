#include "rtRpc.h"
#include "rtRpcClient.h"
#include "rtRpcConfig.h"
#include "rtRpcServer.h"

#include <rtLog.h>
#include <mutex>
#include <thread>

static rtRpcServer* gServer = nullptr;
static std::mutex gMutex;
std::shared_ptr<rtRpcStreamSelector> gStreamSelector;

rtError
rtRpcInit()
{
  rtError e = RT_OK;
  rtRpcConfig::getInstance(true);

  std::lock_guard<std::mutex> lock(gMutex);
  if (gServer == nullptr)
  {
    gServer = new rtRpcServer();
    e = gServer->open();
    if (e != RT_OK)
      rtLogError("failed to open rtRpcServer. %s", rtStrError(e));
  };

  if (gServer == nullptr)
  {
    rtLogError("rtRpcServer is null");
    e = RT_FAIL;
  }

  return e;
}

extern rtError rtRpcShutdownStreamSelector();

rtError
rtRpcShutdown()
{
  rtError e = rtRpcShutdownStreamSelector();
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
rtRpcRegisterObject(char const* id, rtObjectRef const& obj)
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
rtRpcLocateObject(char const* id, rtObjectRef& obj)
{
  if (gServer == nullptr)
  {
    rtLogError("rtRpcInit not called");
    return RT_FAIL;
  }

  if (id == nullptr)
  {
    rtLogError("invalid id (null)");
    return RT_ERROR_INVALID_ARG;
  }

  return gServer->findObject(id, obj, 3000);
}
