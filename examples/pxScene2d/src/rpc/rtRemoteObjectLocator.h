
#include <pthread.h>
#include <stdint.h>
#include <netinet/in.h>
#include <rtObject.h>
#include <map>
#include <string>
#include <rapidjson/document.h>

#include "rtRemoteObjectResolver.h"

class rtRemoteObjectLocator
{
  friend class rtCommandDispatcher;

public:
  rtRemoteObjectLocator();
  ~rtRemoteObjectLocator();

public:
  rtError open(char const* dstaddr, uint16_t dstport, char const* srcaddr);
  rtError start();

  rtError registerObject(std::string const& name, rtObjectRef const& obj);
  rtError findObject(std::string const& name, rtObjectRef& obj, uint32_t timeout = 0);

private:
  struct connected_client
  {
    sockaddr_storage    peer;
    int                 fd;
  };

  typedef rtError (rtRemoteObjectLocator::*command_handler_t)( rapidjson::Document const&,
    sockaddr* soc, socklen_t len);

  typedef std::vector<char> buff_t;

  static void* run_listener(void* argp);

  void run_listener();
  void do_readn(int fd, buff_t& buff);
  void do_accept(int fd);
  void do_dispatch(char const* buff, int n, sockaddr_storage* peer);

  rtError open_rpc_listener();

  // command handlers
  // rtError on_search(rapidjson::Document const& doc, sockaddr* soc, socklen_t len);
  // rtError on_locate(rapidjson::Document const& doc, sockaddr* soc, socklen_t len);

private:
  typedef std::map< std::string, rtObjectRef > refmap_t;
  typedef std::map< std::string, command_handler_t > cmd_handler_map_t;
  typedef std::vector< connected_client > client_list_t;

  sockaddr_storage        m_rpc_endpoint;
  int                     m_rpc_fd;

  refmap_t                m_objects;
  pthread_t               m_thread;

  pthread_cond_t          m_cond;
  pthread_mutex_t         m_mutex;
  cmd_handler_map_t       m_command_handlers;
  client_list_t           m_client_list;
  rtRemoteObjectResolver* m_resolver;
};
