#ifndef __RT_TYPES_H__
#define __RT_TYPES_H__

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include <rtError.h>
#include <rtObject.h>
#include <rapidjson/document.h>

#define kInvalidSocket (-1)

class rtRemoteClient;
class rtRemoteServer;
class rtRemoteConfig;
class rtRemoteStreamSelector;
class rtObjectCache;

struct rtRemoteEnvironment
{
  using client = std::shared_ptr<rtRemoteClient>;
  using message = std::shared_ptr<rapidjson::Document>;

  rtRemoteEnvironment();
  ~rtRemoteEnvironment();

  void shutdown();

  rtRemoteConfig*           Config;
  rtRemoteServer*           Server;
  rtObjectCache*            ObjectCache;
  rtRemoteStreamSelector*   StreamSelector;

  uint32_t RefCount;
  bool     Initialized;

  void enqueueWorkItem(client const& clnt, message const& msg);
  rtError processSingleWorkItem(std::chrono::milliseconds timeout);

private:
  struct WorkItem
  {
    std::shared_ptr<rtRemoteClient> Client;
    std::shared_ptr<rapidjson::Document> Message;
  };

  std::mutex              m_queue_mutex;
  std::condition_variable m_queue_cond;
  std::queue<WorkItem>    m_queue;
};

using rtRemoteEnvPtr = rtRemoteEnvironment*;

using rtSocketBuffer = std::vector<char>;
using rtJsonDocPtr = std::shared_ptr< rapidjson::Document >;
using rtCorrelationKey = uint32_t;
using rtRemoteMessageHandler = std::function<rtError (std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& msg)>;
using rtRemoteInactivityHandler = std::function<rtError(time_t lastMessageTime, time_t now)>;

#endif
