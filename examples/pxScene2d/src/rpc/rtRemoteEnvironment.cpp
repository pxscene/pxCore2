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
{
  StreamSelector = new rtRemoteStreamSelector();
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
rtRemoteEnvironment::registerResponseHandler(MessageHandler handler, void* argp, rtRemoteCorrelationKey k)
{
  rtRemoteCallback<MessageHandler> callback;
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
rtRemoteEnvironment::removeResponseHandler(rtRemoteCorrelationKey k)
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
