#ifndef __RT_REMOTE_OBJECT_RESOLVER_H__
#define __RT_REMOTE_OBJECT_RESOLVER_H__

#include <rtError.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <rapidjson/document.h>

class rtRemoteObjectResolver
{
public:
  rtRemoteObjectResolver(sockaddr_storage const& rpc_endpoint);
  ~rtRemoteObjectResolver();

public:
  rtError open(char const* dstaddr, uint16_t dstport, char const* srcaddr);
  rtError start();

  rtError registerObject(std::string const& name);
  rtError resolveObject(std::string const& name, sockaddr_storage& endpoint, uint32_t timeout = -1);

private:
  static void* run_listener(void* argp);

private:
  typedef rtError (rtRemoteObjectResolver::*command_handler_t)( rapidjson::Document const&, sockaddr* soc, socklen_t len);
  typedef std::vector< char > buff_t;
  typedef std::map< std::string, command_handler_t > cmd_handler_map_t;
  typedef std::set< std::string > object_id_set_t;

  void run_listener();
  void do_read(int fd, buff_t& buff);
  void do_dispatch(char const* buff, int n, sockaddr_storage* peer);

  rtError open_unicast_socket();
  rtError open_multicast_socket();

  // command handlers
  rtError on_search(rapidjson::Document const& doc, sockaddr* soc, socklen_t len);
  rtError on_locate(rapidjson::Document const& doc, sockaddr* soc, socklen_t len);

private:
  sockaddr_storage  m_mcast_dest;
  sockaddr_storage  m_mcast_src;
  int               m_mcast_fd;

  sockaddr_storage  m_ucast_endpoint;
  int               m_ucast_fd;
  socklen_t         m_ucast_len;

  pthread_t         m_read_thread;
  bool              m_read_run;
  bool              m_response_available;

  pthread_cond_t    m_cond;
  pthread_mutex_t   m_mutex;
  pid_t             m_pid;
  cmd_handler_map_t m_command_handlers;
  std::string       m_rpc_addr;
  uint16_t          m_rpc_port;
  object_id_set_t   m_registered_objects;
};

#endif
