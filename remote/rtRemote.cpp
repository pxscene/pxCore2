/*

pxCore Copyright 2005-2018 John Robinson

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

#include "rtRemote.h"
#include "rtRemoteCallback.h"
#include "rtRemoteClient.h"
#include "rtRemoteConfig.h"
#include "rtRemoteMessageHandler.h"
#include "rtRemoteObjectCache.h"
#include "rtRemoteServer.h"
#include "rtRemoteStream.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteConfigBuilder.h"

#include <chrono>
#include <mutex>
#include <thread>


#include <rtLog.h>
#include <unistd.h>

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
    else
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

extern rtError rtRemoteShutdownStreamSelector();

rtError
rtRemoteShutdown(rtRemoteEnvironment* env, bool immediate)
{
  rtError e = RT_FAIL;
  std::lock_guard<std::mutex> lock(gMutex);

  if (!--env->RefCount || immediate)
  {
    if (env->RefCount)
      rtLogWarn("immediate shutdown (reference count: %u), deleting", env->RefCount);
    else
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
rtRemoteUnregisterObject(rtRemoteEnvironment* env, char const* id)
{
  if (env == nullptr)
    return RT_ERROR_INVALID_ARG;

  if (id == nullptr)
    return RT_ERROR_INVALID_ARG;

  return env->Server->unregisterObject(id);
}

rtError
rtRemoteLocateObject(rtRemoteEnvironment* env, char const* id, rtObjectRef& obj, int timeout,
        remoteDisconnectedCallback cb, void *cbdata)
{
  if (env == nullptr)
    return RT_ERROR_INVALID_ARG;

  if (id == nullptr)
    return RT_ERROR_INVALID_ARG;

  return env->Server->findObject(id, obj, timeout, cb, cbdata);
}

rtError
rtRemoteUnregisterDisconnectedCallback( rtRemoteEnvironment* env, remoteDisconnectedCallback cb, void *cbdata )
{
    return env->Server->unregisterDisconnectedCallback( cb, cbdata );
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
rtRemoteRun(rtRemoteEnvironment* env, uint32_t timeout, bool wait)
{

  if (env->Config->server_use_dispatch_thread())
    return RT_ERROR_INVALID_OPERATION;

  rtError e = RT_OK;

  auto time_remaining = std::chrono::milliseconds(timeout);

  do
  {
    auto start = std::chrono::steady_clock::now();
    e = env->processSingleWorkItem(time_remaining, wait, nullptr);
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
rtRemoteUnregisterObject(char const* id)
{
  return rtRemoteUnregisterObject(rtEnvironmentGetGlobal(), id);
}

rtError
rtRemoteLocateObject(char const* id, rtObjectRef& obj, int timeout,
        remoteDisconnectedCallback cb, void *cbdata)
{
  return rtRemoteLocateObject(rtEnvironmentGetGlobal(), id, obj, timeout, cb, cbdata);
}

rtError
rtRemoteUnregisterDisconnectedCallback( remoteDisconnectedCallback cb, void *cbdata )
{
    return rtRemoteUnregisterDisconnectedCallback( rtEnvironmentGetGlobal(), cb, cbdata );
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

rtError
rtRemoteRunUntil(rtRemoteEnvironment* env, uint32_t millisecondsFromNow, bool wait)
{
  rtError e = RT_OK;

  bool hasDipatchThread = env->Config->server_use_dispatch_thread();
  if (hasDipatchThread)
  {
    usleep(millisecondsFromNow * 1000);
    (void ) env;
  }
  else
  {
    auto endTime = std::chrono::milliseconds(millisecondsFromNow) + std::chrono::system_clock::now();
    while (endTime > std::chrono::system_clock::now())
    {
      e = rtRemoteRun(env, wait ? millisecondsFromNow : 16, wait);
      if (e != RT_OK && e != RT_ERROR_QUEUE_EMPTY)
        return e;
    }
  }
  return e;
}
