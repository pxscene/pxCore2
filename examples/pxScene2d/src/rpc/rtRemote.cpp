#include "rtRemote.h"
#include "rtRemoteClient.h"
#include "rtRemoteConfig.h"
#include "rtObjectCache.h"
#include "rtRemoteServer.h"
#include "rtRemoteStream.h"

#include <rtLog.h>
#include <mutex>
#include <thread>

static std::mutex gMutex;
static rtRemoteEnvironment* gEnv = nullptr;

rtRemoteEnvironment::rtRemoteEnvironment()
  : Config(nullptr)
  , Server(nullptr)
  , ObjectCache(nullptr)
  , StreamSelector(nullptr)
{
  Config = rtRemoteConfig::getInstance();

  StreamSelector = new rtRemoteStreamSelector();
  StreamSelector->start();

  Server = new rtRemoteServer(this);
  ObjectCache = new rtObjectCache(this);
}

rtRemoteEnvironment::~rtRemoteEnvironment()
{
}

void
rtRemoteEnvironment::shutdown()
{
  std::lock_guard<std::mutex> lock(gMutex);

  if (Server)
  {
    delete Server;
    Server = nullptr;
  }

  if (StreamSelector)
  {
    rtError e = StreamSelector->shutdown();
    if (e != RT_OK)
      rtLogWarn("failed to shutdown StreamSelector. %s", rtStrError(e));

    delete StreamSelector;
    StreamSelector = nullptr;
  }
}

rtError
rtRemoteInit()
{
  return rtRemoteInitWithEnvironment(RT_REMOTE_DEFAULT_ENVIRONMENT);
}

rtError
rtRemoteInitWithEnvironment(rtRemoteEnvironment* env)
{
  rtError e = RT_FAIL;
  std::lock_guard<std::mutex> lock(gMutex);

  if (env == RT_REMOTE_DEFAULT_ENVIRONMENT)
  {
    if (gEnv!= nullptr)
    {
      rtLogInfo("global context is already initialized");
      return RT_OK;
    }

    gEnv = new rtRemoteEnvironment();

    e = gEnv->Server->open();
    if (e != RT_OK)
      rtLogError("failed to open rtRemoteServer. %s", rtStrError(e));
  }
  else
  {
    rtLogError("custom environment currently not supported");
    e = RT_FAIL;
  }

  return e;
}

rtError
rtRemoteShutdown()
{
  if (gEnv != nullptr)
  {
    gEnv->shutdown();
    delete gEnv;
    gEnv = nullptr;
  }

  return RT_OK;
}

rtError
rtRemoteRegisterObject(char const* id, rtObjectRef const& obj)
{
  if (gEnv == nullptr)
    return RT_FAIL;

  if (id == nullptr)
    return RT_ERROR_INVALID_ARG;

  if (!obj)
    return RT_ERROR_INVALID_ARG;

  return gEnv->Server->registerObject(id, obj);
}

rtError
rtRemoteLocateObject(char const* id, rtObjectRef& obj)
{
  if (gEnv == nullptr)
  {
    rtLogError("context is null");
    return RT_FAIL;
  }

  if (id == nullptr)
  {
    rtLogError("invalid id (null)");
    return RT_ERROR_INVALID_ARG;
  }

  return gEnv->Server->findObject(id, obj, 3000);
}

rtError
rtRemoteRuncOnce(uint32_t /*timeout*/)
{
  return RT_OK;
}

rtError
rtRemoteRun(uint32_t /*timeout*/)
{
  return RT_OK;
}

