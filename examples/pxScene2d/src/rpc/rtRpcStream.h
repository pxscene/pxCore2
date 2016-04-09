#ifndef __RT_RPC_STREAM_H__
#define __RT_RPC_STREAM_H__

#include "rtSocketUtils.h"

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

class rtRpcMessage;
class rtRpcRequest;
class rtRpcStreamSelector;

class rtRpcStream : public std::enable_shared_from_this<rtRpcStream>
{
  friend class rtRpcStreamSelector;

public:
  // TODO: replace rtJsonDocPtr_t with rtRpcMessage
  using message_handler = std::function<rtError (rtJsonDocPtr_t const&)>;
  using inactivity_handler = std::function<rtError (time_t lastMessageTime, time_t now)>;

  rtRpcStream(int fd = -1);

  ~rtRpcStream();

  rtRpcStream(rtRpcStream const&) = delete;
  rtRpcStream& operator = (rtRpcStream const&) = delete;

  rtError open();
  rtError close();
  rtError connectTo(sockaddr_storage const& endpoint);
  rtError send(rtRpcMessage const& msg);
  rtError sendRequest(rtRpcRequest const& req, message_handler handler, uint32_t timeout = 1000);
  rtError setMessageCallback(message_handler handler);
  rtError setInactivityCallback(inactivity_handler handler);

private:
  rtError onIncomingMessage(rt_sockbuf_t& buff, time_t now);
  rtError onInactivity(time_t now);
  rtJsonDocPtr_t waitForResponse(int key, uint32_t timeout);

private:
  using rtRequestMap_t = std::map< rtCorrelationKey_t, rtJsonDocPtr_t >;

  int 				m_fd;
  time_t 			m_last_message_time;
  message_handler 		m_message_handler;
  inactivity_handler 		m_inactivity_handler;
  std::mutex			m_mutex;
  std::condition_variable	m_cond;
  rtRequestMap_t		m_requests;
};

#endif
