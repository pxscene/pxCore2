#ifndef __RT_REMOTE_ENVIRONMENT_H__
#define __RT_REMOTE_ENVIRONMENT_H__

#include "rtRemoteCallback.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

class rtRemoteServer;
class rtRemoteConfig;
class rtRemoteStreamSelector;
class rtObjectCache;

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
  rtObjectCache*            ObjectCache;
  rtRemoteStreamSelector*   StreamSelector;


  uint32_t RefCount;
  bool     Initialized;

  void registerResponseHandler(MessageHandler handler, void* argp, rtCorrelationKey k);
  void removeResponseHandler(rtCorrelationKey k);
  void enqueueWorkItem(std::shared_ptr<rtRemoteClient> const& clnt, rtJsonDocPtr const& doc);
  rtError processSingleWorkItem(std::chrono::milliseconds timeout, rtCorrelationKey* key = nullptr);
  rtError waitForResponse(std::chrono::milliseconds timeout, rtCorrelationKey key);

private:
  struct WorkItem
  {
    std::shared_ptr<rtRemoteClient> Client;
    std::shared_ptr<rapidjson::Document> Message;
  };

  enum class ResponseState
  {
    Waiting,
    Dispatched
  };

  using ResponseHandlerMap = std::map< rtCorrelationKey, rtRemoteCallback<MessageHandler> >;
  using ResponseMap = std::map< rtCorrelationKey, ResponseState >;

  void processRunQueue();

  inline bool haveResponse(rtCorrelationKey k) const
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
