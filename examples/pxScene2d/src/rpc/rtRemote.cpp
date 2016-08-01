#include <chrono>
#include <mutex>
#include <thread>

#include "rtRemote.h"
#include "rtRemoteClient.h"
#include "rtRemoteConfig.h"
#include "rtObjectCache.h"
#include "rtRemoteServer.h"
#include "rtRemoteStream.h"
#include "rtRemoteNameService.h"

#include <rtLog.h>
#include <unistd.h>

static rtRemoteNameService* gNs = nullptr;
static std::mutex gMutex;
static rtRemoteEnvironment* gEnv = nullptr;

rtRemoteEnvironment::rtRemoteEnvironment(rtRemoteConfig* config)
  : Config(config)
  , Server(nullptr)
  , ObjectCache(nullptr)
  , StreamSelector(nullptr)
  , RefCount(1)
  , Initialized(false)
  , m_running(false)
{
  StreamSelector = new rtRemoteStreamSelector();
  StreamSelector->start();

  Server = new rtRemoteServer(this);
  ObjectCache = new rtObjectCache(this);
}

rtRemoteEnvironment::~rtRemoteEnvironment()
{
  delete Config;
}

void
rtRemoteEnvironment::start()
{
  m_running = true;
  if (Config->server_use_dispatch_thread())
  {
    rtLogInfo("starting worker thread");
    m_worker.reset(new std::thread(&rtRemoteEnvironment::processRunQueue, this));
  }
}

void
rtRemoteEnvironment::processRunQueue()
{
  std::chrono::milliseconds timeout(5000);

  while (true)
  {
    rtError e = processSingleWorkItem(timeout, nullptr);
    if (e != RT_OK)
    {
      std::unique_lock<std::mutex> lock(m_queue_mutex);
      if (!m_running)
        return;
      lock.unlock();
      if (e != RT_ERROR_TIMEOUT)
        rtLogWarn("error processing queue. %s", rtStrError(e));
    }
  }
}

void
rtRemoteEnvironment::registerResponseHandler(MessageHandler handler, void* argp, rtCorrelationKey k)
{
  Callback<MessageHandler> callback;
  callback.Func = handler;
  callback.Arg = argp;
  auto ret = m_response_handlers.insert(ResponseHandlerMap::value_type(k, callback));
  RT_ASSERT(ret.second);
}

void
rtRemoteEnvironment::removeResponseHandler(rtCorrelationKey k)
{
  std::unique_lock<std::mutex> lock(m_queue_mutex);
  auto itr = m_response_handlers.find(k);
  if (itr != m_response_handlers.end())
    m_response_handlers.erase(itr);
}

void
rtRemoteEnvironment::shutdown()
{
  if (m_worker)
  {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    m_running = false;
    lock.unlock();
    m_queue_cond.notify_all();
    m_worker->join();
  }

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

  if (ObjectCache)
  {
    rtError e = ObjectCache->clear();
    if (e != RT_OK)
      rtLogWarn("failed to clear object cache. %s", rtStrError(e));

    delete ObjectCache;
    ObjectCache = nullptr;
  }
}

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
  // TODO: clean this up for shutdown.
  if (env->Config->server_use_dispatch_thread())
  {
    // return RT_ERROR_INVALID_OPERATION;
    sleep(timeout);
    return RT_OK;
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
rtRemoteEnvironment::processSingleWorkItem(std::chrono::milliseconds timeout, rtCorrelationKey* key)
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

    rtCorrelationKey const k = rtMessage_GetCorrelationKey(*workItem.Message);
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

