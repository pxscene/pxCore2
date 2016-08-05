#include <chrono>
#include <mutex>
#include <thread>

#include "rtRemote.h"
#include "rtRemoteCallback.h"
#include "rtRemoteClient.h"
#include "rtRemoteConfig.h"
#include "rtRemoteObjectCache.h"
#include "rtRemoteServer.h"
#include "rtRemoteStream.h"
#include "rtRemoteNameService.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteConfigBuilder.h"

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
rtRemoteLocateObject(rtRemoteEnvironment* env, char const* id, rtObjectRef& obj)
{
  if (env == nullptr)
    return RT_ERROR_INVALID_ARG;

  if (id == nullptr)
    return RT_ERROR_INVALID_ARG;

  return env->Server->findObject(id, obj, 3000);
}

rtError
rtRemoteRunOnce(rtRemoteEnvironment* env, uint32_t timeout)
{
  // TODO: Is this the right thing to do?
  if (env->Config->server_use_dispatch_thread())
  {
    RT_ASSERT(false);
    return RT_ERROR_INVALID_OPERATION;
  }
  return env->processSingleWorkItem(std::chrono::milliseconds(timeout));
}

rtError
rtRemoteRun(rtRemoteEnvironment* env, uint32_t timeout)
{

  if (env->Config->server_use_dispatch_thread())
    return RT_ERROR_INVALID_OPERATION;

  rtError e = RT_OK;

  auto time_remaining = std::chrono::milliseconds(timeout);
  while ((time_remaining > std::chrono::milliseconds(0)) && (e == RT_OK))
  {
    auto start = std::chrono::steady_clock::now();
    e = env->processSingleWorkItem(time_remaining);
    if (e != RT_OK)
      return e;
    auto end = std::chrono::steady_clock::now();
    time_remaining = std::chrono::milliseconds((end - start).count());
  }

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
rtRemoteEnvironment::waitForResponse(std::chrono::milliseconds timeout, rtRemoteCorrelationKey k)
{
  rtError e = RT_OK;

  auto delay = std::chrono::system_clock::now() + timeout;
  std::unique_lock<std::mutex> lock(m_queue_mutex);
  if (!m_queue_cond.wait_until(lock, delay, [this, k] { return (haveResponse(k) || !m_running); }))
  {
    e = RT_ERROR_TIMEOUT;
  }

  // TODO: Is this the proper code?
  if (!m_running)
    e = RT_FAIL;

  return e;
}

rtError
rtRemoteEnvironment::processSingleWorkItem(std::chrono::milliseconds timeout, rtRemoteCorrelationKey* key)
{
  rtError e = RT_ERROR_TIMEOUT;

  if (key)
    *key = kInvalidCorrelationKey;

  WorkItem workItem;
  auto delay = std::chrono::system_clock::now() + timeout;

  std::unique_lock<std::mutex> lock(m_queue_mutex);
  if (!m_queue_cond.wait_until(lock, delay, [this] { return !this->m_queue.empty() || !m_running; }))
  {
    e = RT_ERROR_TIMEOUT;
  }
  else
  {
    if (!m_running)
      return RT_OK;

    if (!m_queue.empty())
    {
      workItem = m_queue.front();
      m_queue.pop();
    }
  }

  if (workItem.Message)
  {
    MessageHandler messageHandler = nullptr;
    void* argp = nullptr;

    rtRemoteCorrelationKey const k = rtMessage_GetCorrelationKey(*workItem.Message);
    auto itr = m_response_handlers.find(k);
    if (itr != m_response_handlers.end())
    {
      messageHandler = itr->second.Func;
      argp = itr->second.Arg;
      m_response_handlers.erase(itr);
    }
    lock.unlock();

    if (messageHandler != nullptr)
      e = messageHandler(workItem.Client, workItem.Message, argp);
    else
      e = Server->processMessage(workItem.Client, workItem.Message);

    if (key)
      *key = k;

    if (Config->server_use_dispatch_thread())
    {
      std::unique_lock<std::mutex> lock(m_queue_mutex);
      auto itr = m_waiters.find(k);
      if (itr != m_waiters.end())
        itr->second = ResponseState::Dispatched;
      lock.unlock();
      m_queue_cond.notify_all();
    }
  }

  return e;
}

void
rtRemoteEnvironment::enqueueWorkItem(std::shared_ptr<rtRemoteClient> const& clnt,
  rtJsonDocPtr const& doc)
{
  WorkItem workItem;
  workItem.Client = clnt;
  workItem.Message = doc;

  std::unique_lock<std::mutex> lock(m_queue_mutex);
  m_queue.push(workItem);
  lock.unlock();
  m_queue_cond.notify_all();
}

