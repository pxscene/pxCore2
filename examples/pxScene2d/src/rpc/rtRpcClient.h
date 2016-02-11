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

#include "rtRpcTypes.h"
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
  rtError startSession(std::string const& object_id);

  rtError get(std::string const& id, char const* name, rtValue* value);
  rtError set(std::string const& id, char const* name, rtValue const* value);

  rtError get(std::string const& id, uint32_t index, rtValue* value);
  rtError set(std::string const& id, uint32_t index, rtValue const* value);

  rtError send(std::string const& id, std::string const& name, int argc, rtValue const* argv, rtValue* result);

  inline void keep_alive(std::string const& s)
    { m_object_list.push_back(s); }

private:
  typedef uint32_t key_type;

  typedef rtError (rtRpcClient::*message_handler_t)(rtJsonDocPtr_t const&);
  typedef std::map< std::string, message_handler_t > msghandler_map_t;
  typedef std::map< key_type, rtJsonDocPtr_t > request_map_t;

  rtJsonDocPtr_t waitForResponse(int key, uint32_t timeout = 1000);

  rtError connectRpcEndpoint();
  rtError sendKeepAlive();
  rtError runListener();
  rtError readn(int fd, rt_sockbuf_t& buff);
  rtError dispatch(rtJsonDocPtr_t const& doc);

  // message handlers
  rtError onStartSession(rtJsonDocPtr_t const& doc);

private:
  sockaddr_storage              m_remote_endpoint;
  int                           m_fd;
  std::vector<std::string>      m_object_list;
  std::mutex                    m_mutex;
  std::unique_ptr<std::thread>  m_thread;
  std::condition_variable       m_cond;
  msghandler_map_t              m_message_handlers;
  std::atomic<key_type>         m_next_key;
  request_map_t                 m_requests;
};

#endif
