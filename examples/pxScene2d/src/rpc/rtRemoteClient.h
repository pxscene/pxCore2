#ifndef __RT_RPC_TRANSPORT_H__
#define __RT_RPC_TRANSPORT_H__

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <rtError.h>
#include <rtValue.h>

#include <sys/socket.h>
#include <rapidjson/document.h>

#include "rtRemoteStream.h"
#include "rtRemoteTypes.h"
#include "rtRemoteMessage.h"
#include "rtSocketUtils.h"

class rtRemoteClient: public std::enable_shared_from_this<rtRemoteClient>
{
public:
  rtRemoteClient(int fd, sockaddr_storage const& local_endpoint, sockaddr_storage const& remote_endpoint);
  rtRemoteClient(sockaddr_storage const& remote_endpoint);
  ~rtRemoteClient();

  rtError open();
  rtError startSession(std::string const& objectName, uint32_t timeout = 0);
  rtError get(std::string const& objectName, char const* propertyName, rtValue& value, uint32_t timeout = 0);
  rtError get(std::string const& objectName, uint32_t index, rtValue& value, uint32_t timeout = 0);
  rtError set(std::string const& objectName, uint32_t index, rtValue const& value, uint32_t timeout = 0);
  rtError set(std::string const& objectName, char const* propertyName, rtValue const& value, uint32_t timeout = 0);

  rtError send(std::string const& objectName, std::string const& name, int argc, rtValue const* argv,
    rtValue* result, uint32_t timeout);

  inline void keepAlive(std::string const& s)
    { m_object_list.push_back(s); }

  rtError setMessageCallback(rtRemoteMessageHandler const& handler)
    { m_message_handler = handler; return RT_OK; } 

  rtError sendDocument(rapidjson::Document const& doc)
  {
    return m_stream->sendDocument(doc);
  }

  inline sockaddr_storage getRemoteEndpoint() const
    { return m_stream->getRemoteEndpoint(); }

  inline sockaddr_storage getLocalEndpoint() const
    { return m_stream->getLocalEndpoint(); }

private:

  rtError onIncomingMessage(rtJsonDocPtr const& msg);
  rtError onInactivity(time_t lastMessage, time_t now);
  rtError sendGet(rtRemoteGetRequest const& req, rtValue& value, uint32_t timeout);
  rtError sendSet(rtRemoteSetRequest const& req, uint32_t timeout);
  rtError connectRpcEndpoint();
  rtError sendKeepAlive();
  rtError runListener();
  rtError dispatch(rtJsonDocPtr const& doc);

  // message handlers
  rtError onStartSession(rtJsonDocPtr const& doc);

private:
  std::shared_ptr<rtRemoteStream>   m_stream;
  std::vector<std::string>          m_object_list;
  rtRemoteMessageHandler            m_message_handler;
};

#endif
