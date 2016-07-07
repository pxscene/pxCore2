#ifndef __RT_RPC_STREAM_H__
#define __RT_RPC_STREAM_H__

#include "rtRemoteTypes.h"
#include "rtSocketUtils.h"

#include <condition_variable>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

class rtRemoteClient;
class rtRemoteMessage;
class rtRemoteRequest;
class rtRemoteStreamSelector;

class rtRemoteStream : public std::enable_shared_from_this<rtRemoteStream>
{
  friend class rtRemoteStreamSelector;
  friend class rtRemoteClient;

public:
  using MessageHandler = std::function<rtError (rtJsonDocPtr const& doc)>;

  rtRemoteStream(rtRemoteEnvPtr env, int fd,
    sockaddr_storage const& local_endpoint, sockaddr_storage const& remote_endpoint);
  ~rtRemoteStream();

  rtRemoteStream(rtRemoteStream const&) = delete;
  rtRemoteStream& operator = (rtRemoteStream const&) = delete;

  inline bool isConnected() const
    { return m_fd != -1; }

  rtError open();
  rtError close();
  rtError connectTo(sockaddr_storage const& endpoint);
  rtError connect();
  rtError send(rtRemoteMessage const& msg);
  rtError sendRequest(rtRemoteRequest const& req, MessageHandler handler, uint32_t timeout = 1000);
  rtError setMessageCallback(MessageHandler handler);
  rtError setInactivityCallback(rtRemoteInactivityHandler handler);

  inline sockaddr_storage getLocalEndpoint() const
    { return m_local_endpoint; }

  inline sockaddr_storage getRemoteEndpoint() const
    { return m_remote_endpoint; }

private:
  rtError sendDocument(rapidjson::Document const& doc)
  {
    std::unique_lock<std::mutex> lock(m_send_mutex);
    return rtSendDocument(doc, m_fd, nullptr);
  }

  rtError onIncomingMessage(rtSocketBuffer& buff, time_t now);
  rtError onInactivity(time_t now);
  rtJsonDocPtr waitForResponse(int key, uint32_t timeout);

private:
  void runMessageDispatch();

  struct WorkItem
  {
    rtJsonDocPtr  Doc;
  };

  using rtRequestMap = std::map< rtCorrelationKey, rtJsonDocPtr >;

  int                       m_fd;
  time_t                    m_last_message_time;
  time_t                    m_last_ka_message_time;
  MessageHandler            m_message_handler;
  rtRemoteInactivityHandler m_inactivity_handler;
  std::mutex                m_mutex;
  std::condition_variable   m_cond;
  rtRequestMap              m_requests;
  sockaddr_storage          m_local_endpoint;
  sockaddr_storage          m_remote_endpoint;

  // work queue
  std::vector<std::thread*> m_dispatch_threads;
  bool                      m_running;
  std::deque<WorkItem>      m_work_queue;
  std::mutex                m_work_mutex;
  std::condition_variable   m_work_cond;

  std::mutex                m_send_mutex;
  rtRemoteEnvPtr            m_env;
};

class rtRemoteStreamSelector
{
public:
  rtRemoteStreamSelector();

  rtError start();
  rtError registerStream(std::shared_ptr<rtRemoteStream> const& s);
  rtError shutdown();

private:
  rtError pollFds();

private:
  std::vector< std::shared_ptr<rtRemoteStream> >  m_streams;
  std::shared_ptr< std::thread >                  m_thread;
  std::mutex                                      m_mutex;
  int                                             m_shutdown_pipe[2];
};


#endif
