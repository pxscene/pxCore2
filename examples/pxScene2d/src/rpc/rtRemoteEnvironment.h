#ifndef __RT_REMOTE_ENVIRONMENT_H__
#define __RT_REMOTE_ENVIRONMENT_H__

#include "rtRemoteCallback.h"
#include "rtRemoteCorrelationKey.h"
#include "rtRemoteMessageHandler.h"

#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

class rtRemoteServer;
class rtRemoteConfig;
class rtRemoteStreamSelector;
class rtRemoteObjectCache;

struct rtRemoteEnvironment
{
  rtRemoteEnvironment(rtRemoteConfig* config);
  ~rtRemoteEnvironment();

  void shutdown();
  void start();
  bool isQueueEmpty() const
  {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    return m_queue.empty();
  }

  rtRemoteConfig const*     Config;
  rtRemoteServer*           Server;
  rtRemoteObjectCache*      ObjectCache;
  rtRemoteStreamSelector*   StreamSelector;


  uint32_t RefCount;
  bool     Initialized;

  void registerResponseHandler(rtRemoteMessageHandler handler, void* argp, rtRemoteCorrelationKey k);
  void removeResponseHandler(rtRemoteCorrelationKey k);
  void enqueueWorkItem(std::shared_ptr<rtRemoteClient> const& clnt, rtRemoteMessagePtr const& doc);
  rtError processSingleWorkItem(rtRemoteCorrelationKey* key = nullptr, bool blocking = false, uint32_t delay = 0);
  rtError waitForResponse(std::chrono::milliseconds timeout, rtRemoteCorrelationKey key);

private:
  struct WorkItem
  {
    std::shared_ptr<rtRemoteClient> Client;
    std::shared_ptr<rtRemoteMessage> Message;
  };

  enum class ResponseState
  {
    Waiting,
    Dispatched
  };

  using ResponseHandlerMap = std::map< rtRemoteCorrelationKey, rtRemoteCallback<rtRemoteMessageHandler> >;
  using ResponseMap = std::map< rtRemoteCorrelationKey, ResponseState >;

  void processRunQueue();

  inline bool haveResponse(rtRemoteCorrelationKey k) const
    { return m_waiters.find(k) != m_waiters.end(); }

  mutable std::mutex            m_queue_mutex;
  std::condition_variable       m_queue_cond;
  std::queue<WorkItem>          m_queue;
  std::unique_ptr<std::thread>  m_worker;
  bool                          m_running;
  ResponseHandlerMap            m_response_handlers;
  ResponseMap                   m_waiters;
};

#endif
