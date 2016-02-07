#ifndef __RT_RPC_TRANSPORT_H__
#define __RT_RPC_TRANSPORT_H__

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <rtAtomic.h>
#include <rtError.h>
#include <rtValue.h>

#include <sys/socket.h>
#include <pthread.h>
#include <rapidjson/document.h>

#include "rtRpcTypes.h"
#include "rtSocketUtils.h"

class rtValueWriter
{
  static rtError write(rtValue const& val, rtJsonDocPtr_t const& doc);
};

class rtValueReader
{
public:
  static rtError read(rtValue& val, rtJsonDocPtr_t const& doc);
};

class rtRpcTransport
{
public:
  rtRpcTransport(sockaddr_storage const& ss);
  ~rtRpcTransport();

  rtError start();
  rtError start_session(std::string const& object_id);

  rtError get(std::string const& id, char const* name, rtValue* value);
  rtError set(std::string const& id, char const* name, rtValue const* value);

  rtError get(std::string const& id, uint32_t index, rtValue* value);
  rtError set(std::string const& id, uint32_t index, rtValue const* value);

  inline void keep_alive(std::string const& s)
    { m_object_list.push_back(s); }

private:
  typedef rtError (rtRpcTransport::*message_handler_t)(rtJsonDocPtr_t const&);
  typedef std::map< std::string, message_handler_t > msghandler_map_t;
  typedef std::map< rtAtomic, rtJsonDocPtr_t > request_map_t;

  uint32_t next_key()
    { return rtAtomicInc(&m_corkey); }

  rtJsonDocPtr_t wait_for_response(int key);

  rtError connect_rpc_endpoint();
  rtError send_keep_alive();
  rtError run_listener();
  rtError readn(int fd, rt_sockbuf_t& buff);
  rtError dispatch(rtJsonDocPtr_t const& doc);

  // message handlers
  rtError on_start_session(rtJsonDocPtr_t const& doc);

  static void* run_listener(void* argp);

private:
  sockaddr_storage          m_remote_endpoint;
  int                       m_fd;
  std::vector<std::string>  m_object_list;
  pthread_t                 m_thread;
  pthread_mutex_t           m_mutex;
  pthread_cond_t            m_cond;
  msghandler_map_t          m_message_handlers;
  rtAtomic                  m_corkey;
  request_map_t             m_requests;
};

#endif
