
#include <pthread.h>
#include <stdint.h>
#include <netinet/in.h>
#include <rtObject.h>
#include <map>
#include <string>
#include <rapidjson/document.h>

class rtRemoteObjectLocator
{
  friend class rtCommandDispatcher;

public:
  rtRemoteObjectLocator();
  ~rtRemoteObjectLocator();

public:
  rtError open(char const* dstaddr, uint16_t dstport, char const* srcaddr);
  rtError registerObject(std::string const& name, rtObjectRef const& obj);
  rtError startListener(bool serverMode = false);
  rtObjectRef findObject(std::string const& name, uint32_t timeout = -1);

private:
  static void* run_listener(void* argp);

private:
  struct connected_client
  {
    sockaddr_storage    peer;
    int                 fd;
  };

  typedef rtError (rtRemoteObjectLocator::*command_handler_t)( rapidjson::Document const&,
    sockaddr* soc, socklen_t len);

  void run_listener();
  void do_read(int fd);
  void do_accept(int fd);

  rtError open_unicast_socket();
  rtError open_multicast_socket();
  rtError open_rpc_listener();

  // command handlers
  rtError on_search(rapidjson::Document const& doc, sockaddr* soc, socklen_t len);
  rtError on_locate(rapidjson::Document const& doc, sockaddr* soc, socklen_t len);

private:
  typedef std::map< std::string, rtObjectRef > refmap_t;
  typedef std::vector<char> buff_t;
  typedef std::map< std::string, command_handler_t > cmd_handler_map_t;
  typedef std::vector< connected_client > client_list_t;

  sockaddr_storage  m_mcast_dest;
  sockaddr_storage  m_mcast_src;
  int               m_mcast_fd;

  sockaddr_storage  m_ucast_endpoint;
  int               m_ucast_fd;

  sockaddr_storage  m_rpc_endpoint;
  int               m_rpc_fd;

  refmap_t          m_objects;
  pthread_t         m_read_thread;
  buff_t            m_read_buff;
  bool              m_read_run;
  bool              m_response_available;

  pthread_cond_t    m_cond;
  pthread_mutex_t   m_mutex;
  pid_t             m_pid;
  cmd_handler_map_t m_command_handlers;
  client_list_t     m_client_list;
};
