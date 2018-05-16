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

class rtRemoteEnvironment
{
public:
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

  using rtRemoteQueueReady = void (*)(void*);

  uint32_t RefCount;
  bool     Initialized;

  void registerQueueReadyHandler(rtRemoteQueueReady handler, void* argp);
  void registerResponseHandler(rtRemoteMessageHandler handler, void* argp, rtRemoteCorrelationKey const& k);
  void removeResponseHandler(rtRemoteCorrelationKey const& k);
  void enqueueWorkItem(std::shared_ptr<rtRemoteClient> const& clnt, rtRemoteMessagePtr const& doc);
  rtError processSingleWorkItem(std::chrono::milliseconds timeout, bool wait, rtRemoteCorrelationKey* key, const rtRemoteCorrelationKey* const specificKey = nullptr);
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
  using WorkItemMap = std::map< rtRemoteCorrelationKey, WorkItem >;

  void processRunQueue();

  inline bool haveResponse(rtRemoteCorrelationKey k) const
  {
    auto itr = m_waiters.find(k);
    return (itr != m_waiters.end()) && (itr->second == ResponseState::Dispatched);
  }

  using thread_ptr = std::unique_ptr<std::thread>;

  mutable std::mutex            m_queue_mutex;
  std::condition_variable       m_queue_cond;
  std::queue<WorkItem>          m_queue;
  std::vector< thread_ptr >     m_workers;
  bool                          m_running;
  ResponseHandlerMap            m_response_handlers;
  ResponseMap                   m_waiters;
  WorkItemMap                   m_specific_workitem_map; // Store workitems for pending requests here, not m_queue
  rtRemoteQueueReady            m_queue_ready_handler;
  void*                         m_queue_ready_context;
};

#endif
