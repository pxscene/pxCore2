#ifndef __RT_REMOTE_OBJECT_RESOLVER_H__
#define __RT_REMOTE_OBJECT_RESOLVER_H__

#include <rtAtomic.h>
#include <rtError.h>

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <sys/socket.h>

#include "rtRpcTypes.h"

class rtRemoteObjectResolver
{
public:
  rtRemoteObjectResolver(sockaddr_storage const& rpc_endpoint);
  ~rtRemoteObjectResolver();

public:
  rtError open(char const* dstaddr, uint16_t dstport, char const* srcaddr);
  rtError start();

  rtError registerObject(std::string const& name);
  rtError resolveObject(std::string const& name, sockaddr_storage& endpoint, uint32_t timeout = 1000);

private:
  typedef rtError (rtRemoteObjectResolver::*command_handler_t)(rtJsonDocPtr_t const&, sockaddr_storage const& soc);
  typedef std::vector< char > buff_t;
  typedef std::map< std::string, command_handler_t > cmd_handler_map_t;
  typedef std::map< rtAtomic, rtJsonDocPtr_t > request_map_t;
  typedef std::set< std::string > object_id_set_t;

  void runListener();
  void doRead(int fd, buff_t& buff);
  void doDispatch(char const* buff, int n, sockaddr_storage* peer);

  rtError openUnicastSocket();
  rtError openMulticastSocket();

  // command handlers
  rtError onSearch(rtJsonDocPtr_t const& doc, sockaddr_storage const& soc);
  rtError onLocate(rtJsonDocPtr_t const& doc, sockaddr_storage const& soc);

private:
  sockaddr_storage  m_mcast_dest;
  sockaddr_storage  m_mcast_src;
  int               m_mcast_fd;
  uint32_t          m_mcast_src_index;

  sockaddr_storage  m_ucast_endpoint;
  int               m_ucast_fd;
  socklen_t         m_ucast_len;

  std::unique_ptr<std::thread> m_read_thread;
  std::condition_variable m_cond;
  std::mutex        m_mutex;
  pid_t             m_pid;
  cmd_handler_map_t m_command_handlers;
  std::string       m_rpc_addr;
  uint16_t          m_rpc_port;
  object_id_set_t   m_registered_objects;
  rtAtomic          m_seq_id;
  request_map_t     m_pending_searches;
};

#endif
