#include "rtRemote.h"
#include "rtRemoteCallback.h"
#include "rtRemoteClient.h"
#include "rtRemoteConfig.h"
#include "rtRemoteMessageHandler.h"
#include "rtRemoteObjectCache.h"
#include "rtRemoteServer.h"
#include "rtRemoteStream.h"
#include "rtRemoteNameService.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteConfigBuilder.h"

#include <chrono>
#include <mutex>
#include <thread>


#include <rtLog.h>
#include <unistd.h>

static rtRemoteNameService* gNs = nullptr;
static std::mutex gMutex;
static rtRemoteEnvironment* gEnv = nullptr;

rtError
rtRemoteInit(rtRemoteEnvironment* env)
{
  rtError e = RT_FAIL;
  std::lock_guard<std::mutex> lock(gMutex);

  rtLogDebug("initialize environment: %p", env);
  if (!env->Initialized)
  {
    rtLogDebug("environment: %p not initialized, opening server", env);
    e = env->Server->open();
    if (e != RT_OK)
      rtLogError("failed to open rtRemoteServer. %s", rtStrError(e));

    env->start();
  }
  else
  {
    env->RefCount++;
    e = RT_OK;
  }

  if (e == RT_OK)
  {
    rtLogDebug("environment is now initialized: %p", env);
    env->Initialized = true;
  }

  return e;
}

rtError
rtRemoteInitNs(rtRemoteEnvironment* env)
{
  rtError e = RT_OK;
  //rtRemoteConfig::getInstance();
  if (gNs == nullptr)
  {
    gNs = new rtRemoteNameService(env);
    e = gNs->init();
  }

  return e;
}

extern rtError rtRemoteShutdownStreamSelector();

rtError
rtRemoteShutdown(rtRemoteEnvironment* env)
{
  rtError e = RT_FAIL;
  std::lock_guard<std::mutex> lock(gMutex);

  env->RefCount--;
  if (env->RefCount == 0)
  {
    rtLogInfo("environment reference count is zero, deleting");
    env->shutdown();
    if (env == gEnv)
      gEnv = nullptr;
    delete env;
    e = RT_OK;
  }
  else
  {
    rtLogInfo("environment reference count is non-zero. %u", env->RefCount);
    e = RT_OK;
  }

  return e;
}

rtError
rtRemoteShutdownNs()
{
  if (gNs)
  {
    delete gNs;
    gNs = nullptr;
  }
  return RT_OK;
}

rtError
rtRemoteRegisterObject(rtRemoteEnvironment* env, char const* id, rtObjectRef const& obj)
{
  if (env == nullptr)
    return RT_FAIL;

  if (id == nullptr)
    return RT_ERROR_INVALID_ARG;

  if (!obj)
    return RT_ERROR_INVALID_ARG;

  return env->Server->registerObject(id, obj);
}

rtError
rtRemoteLocateObject(rtRemoteEnvironment* env, char const* id, rtObjectRef& obj, int timeout)
{
  if (env == nullptr)
    return RT_ERROR_INVALID_ARG;

  if (id == nullptr)
    return RT_ERROR_INVALID_ARG;

  return env->Server->findObject(id, obj, timeout);
}

rtError
rtRemoteRegisterQueueReadyHandler ( rtRemoteEnvironment* env, rtRemoteQueueReady handler, void* argp)
{
  env->registerQueueReadyHandler(handler, argp);
  return RT_OK;
}

rtError
rtRemoteProcessSingleItem(rtRemoteEnvironment* env)
{
  using namespace std::chrono;
  const uint32_t dummy_timeout_to_guarantee_one_item_processing = 0;

  return env->processSingleWorkItem(milliseconds(dummy_timeout_to_guarantee_one_item_processing), false, nullptr);
};

rtError
rtRemoteRun(rtRemoteEnvironment* env, uint32_t timeout)
{

  if (env->Config->server_use_dispatch_thread())
    return RT_ERROR_INVALID_OPERATION;

  rtError e = RT_OK;

  auto time_remaining = std::chrono::milliseconds(timeout);

  do
  {
    auto start = std::chrono::steady_clock::now();
    e = env->processSingleWorkItem(time_remaining, false, nullptr);
    if (e != RT_OK)
      return e;
    auto end = std::chrono::steady_clock::now();
    time_remaining -= std::chrono::milliseconds((end - start).count());
  }
  while ((time_remaining > std::chrono::milliseconds(0)) && (e == RT_OK));

  return e;
}

rtRemoteEnvironment*
rtEnvironmentFromFile(char const* configFile)
{
  RT_ASSERT(configFile != nullptr);

  rtRemoteConfigBuilder* builder(rtRemoteConfigBuilder::fromFile(configFile));
  rtRemoteEnvironment* env(new rtRemoteEnvironment(builder->build()));
  delete builder;

  return env;

}

rtRemoteEnvironment*
rtEnvironmentGetGlobal()
{
  std::lock_guard<std::mutex> lock(gMutex);
  if (gEnv == nullptr)
  {
    rtRemoteConfigBuilder* builder = rtRemoteConfigBuilder::getDefaultConfig();
    rtRemoteConfig* conf = builder->build();
    gEnv = new rtRemoteEnvironment(conf);
    delete builder;

  }
  return gEnv;
}

rtError
rtRemoteInit()
{
  return rtRemoteInit(rtEnvironmentGetGlobal());
}

rtError
rtRemoteRegisterObject(char const* id, rtObjectRef const& obj)
{
  return rtRemoteRegisterObject(rtEnvironmentGetGlobal(), id, obj);
}

rtError
rtRemoteLocateObject(char const* id, rtObjectRef& obj, int timeout)
{
  return rtRemoteLocateObject(rtEnvironmentGetGlobal(), id, obj, timeout);
}

rtError
rtRemoteShutdown()
{
  return rtRemoteShutdown(rtEnvironmentGetGlobal());
}

rtError
rtRemoteProcessSingleItem()
{
  return rtRemoteProcessSingleItem(rtEnvironmentGetGlobal());
}
