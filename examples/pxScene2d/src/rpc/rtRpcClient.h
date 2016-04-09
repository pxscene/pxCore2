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

#include "rtRpcStream.h"
#include "rtRpcTypes.h"
#include "rtRpcMessage.h"
#include "rtSocketUtils.h"

class rtRpcClient;

class rtValueWriter
{
public:
  static rtError write(rtValue const& from, rapidjson::Value& to, rapidjson::Document& parent);
};

class rtValueReader
{
public:
  static rtError read(rtValue& val, rapidjson::Value const& from,
    std::shared_ptr<rtRpcClient> const& tport = std::shared_ptr<rtRpcClient>());
};

class rtRpcClient: public std::enable_shared_from_this<rtRpcClient>
{
public:
  rtRpcClient(sockaddr_storage const& ss);
  ~rtRpcClient();

  rtError start();
  rtError startSession(std::string const& objectName, uint32_t timeout = kDefaultRequestTimeout);

  rtError get(std::string const& objectName, char const* propertyName, rtValue& value,
    uint32_t timeout = kDefaultRequestTimeout);

  rtError set(std::string const& objectName, char const* propertyName, rtValue const& value,
    uint32_t timeout = kDefaultRequestTimeout);

  rtError get(std::string const& objectName, uint32_t index, rtValue& value,
    uint32_t timeout = kDefaultRequestTimeout);

  rtError set(std::string const& objectName, uint32_t index, rtValue const& value,
    uint32_t timeout = kDefaultRequestTimeout);

  rtError send(std::string const& objectName, std::string const& name, int argc, rtValue const* argv,
    rtValue* result, uint32_t timeout = kDefaultRequestTimeout);

  inline void keep_alive(std::string const& s)
    { m_object_list.push_back(s); }

private:

  rtError onIncomingMessage(rtJsonDocPtr_t const& doc);
  rtError onInactivity(time_t lastMessage, time_t now);
  rtError sendGet(rtRpcGetRequest const& req, rtValue& value, uint32_t timeout);
  rtError sendSet(rtRpcSetRequest const& req, uint32_t timeout);
  rtError connectRpcEndpoint();
  rtError sendKeepAlive();
  rtError runListener();
  rtError dispatch(rtJsonDocPtr_t const& doc);

  // message handlers
  rtError onStartSession(rtJsonDocPtr_t const& doc);

private:
  sockaddr_storage              	m_remote_endpoint;
  std::shared_ptr<rtRpcStream>		m_stream;
  std::vector<std::string>      	m_object_list;
  std::atomic<rtCorrelationKey_t>	m_next_key;
};

#endif
