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

#include "rtRemoteEnvironment.h"
#include "rtRemoteConfig.h"
#include "rtRemoteServer.h"
#include "rtRemoteStreamSelector.h"
#include "rtRemoteObjectCache.h"
#include "rtError.h"

rtRemoteEnvironment::rtRemoteEnvironment(rtRemoteConfig* config)
  : Config(config)
  , Server(nullptr)
  , ObjectCache(nullptr)
  , StreamSelector(nullptr)
  , RefCount(1)
  , Initialized(false)
  , m_running(false)
  , m_queue_ready_handler(nullptr)
  , m_queue_ready_context(nullptr)
{
  StreamSelector = new rtRemoteStreamSelector(this);
  StreamSelector->start();

  Server = new rtRemoteServer(this);
  ObjectCache = new rtRemoteObjectCache(this);
}

rtRemoteEnvironment::~rtRemoteEnvironment()
{
  delete Config;
}

void
rtRemoteEnvironment::start()
{
  static int const kNumWorkers = 4;

  m_running = true;
  if (Config->server_use_dispatch_thread())
  {
    while (m_workers.size() < kNumWorkers)
    {
      rtLogInfo("starting worker thread");
      thread_ptr p(new std::thread(&rtRemoteEnvironment::processRunQueue, this));
      m_workers.push_back(std::move(p));
    }
  }
}

void
rtRemoteEnvironment::processRunQueue()
{
  std::chrono::milliseconds timeout(5000);

  while (true)
  {
    rtError e = processSingleWorkItem(timeout, true,  nullptr);
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
rtRemoteEnvironment::registerResponseHandler(rtRemoteMessageHandler handler, void* argp, rtRemoteCorrelationKey const& k)
{
  rtRemoteCallback<rtRemoteMessageHandler> callback;
  callback.Func = handler;
  callback.Arg = argp;

  std::unique_lock<std::mutex> lock(m_queue_mutex);

  {
    auto ret = m_response_handlers.insert(ResponseHandlerMap::value_type(k, callback));
    if (!ret.second)
      rtLogError("callback for %s already exists", k.toString().c_str());
    RT_ASSERT(ret.second);
  }

  // prime response map. An entry in this map indicates that a caller is waiting
  // for a response
  if (Config->server_use_dispatch_thread())
  {
    auto ret = m_waiters.insert(ResponseMap::value_type(k, ResponseState::Waiting));
    if (!ret.second)
      rtLogWarn("response indicator for %s already exists", k.toString().c_str());
    RT_ASSERT(ret.second);
  }
}

void
rtRemoteEnvironment::removeResponseHandler(rtRemoteCorrelationKey const& k)
{
  std::unique_lock<std::mutex> lock(m_queue_mutex);
  {
    auto itr = m_response_handlers.find(k);
    if (itr != m_response_handlers.end())
      m_response_handlers.erase(itr);
  }

  if (Config->server_use_dispatch_thread())
  {
    auto itr = m_waiters.find(k);
    if (itr != m_waiters.end())
      m_waiters.erase(itr);
  }
}

void
rtRemoteEnvironment::shutdown()
{
  std::unique_lock<std::mutex> lock(m_queue_mutex);
  m_running = false;
  lock.unlock();
  m_queue_cond.notify_all();

  for (auto& t : m_workers)
    t->join();

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
rtRemoteEnvironment::processSingleWorkItem(std::chrono::milliseconds timeout, bool wait, rtRemoteCorrelationKey* key)
{
  rtError e = RT_ERROR_TIMEOUT;

  if (key)
    *key = kInvalidCorrelationKey;

  WorkItem workItem;
  auto delay = std::chrono::system_clock::now() + timeout;

  std::unique_lock<std::mutex> lock(m_queue_mutex);
  if (!wait && m_queue.empty())
    return RT_ERROR_QUEUE_EMPTY;

  //
  // TODO: if someone is already waiting for a specifc response
  // then we should use a 2nd queue
  //

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
    rtRemoteMessageHandler messageHandler = nullptr;
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
  rtRemoteMessagePtr const& doc)
{
  WorkItem workItem;
  workItem.Client = clnt;
  workItem.Message = doc;

  std::unique_lock<std::mutex> lock(m_queue_mutex);

  #if 0
  // TODO if someone is waiting for this message, then deliver it directly
  rtRemoteCorrelationKey const k = rtMessage_GetCorrelationKey(*workItem.Message);
  auto itr = m_response_handlers.find(k);
  if (itr != m_response_handlers.end())
  {
    rtError e = itr->second.Func(workItem.Client, workItem.Message, itr->second.Arg);
    if (e != RT_OK)
      rtLogWarn("error dispatching message directly to waiter. %s", rtStrError(e));
  }
  else
  {
    m_queue.push(workItem);
    m_queue_cond.notify_all();
  }
  #endif

  m_queue.push(workItem);
  lock.unlock();
  m_queue_cond.notify_all();

  if (m_queue_ready_handler != nullptr)
  {
    m_queue_ready_handler(m_queue_ready_context);
  }
}

void
rtRemoteEnvironment::registerQueueReadyHandler(rtRemoteQueueReady handler, void* argp)
{
  m_queue_ready_handler = handler;
  m_queue_ready_context = argp;
}
