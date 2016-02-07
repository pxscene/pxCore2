
#include <pthread.h>
#include <stdint.h>
#include <netinet/in.h>
#include <rtObject.h>
#include <map>
#include <string>
#include <rapidjson/document.h>

#include "rtRpcTypes.h"
#include "rtRemoteObjectResolver.h"
#include "rtRpcTransport.h"
#include "rtSocketUtils.h"

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

  typedef rtError (rtRemoteObjectLocator::*command_handler_t)(rtJsonDocPtr_t const&, sockaddr_storage const& soc);

  static void* run_listener(void* argp);

  void run_listener();
  rtError do_readn(int fd, rt_sockbuf_t& buff, sockaddr_storage const& peer);
  void do_accept(int fd);
  void do_dispatch(rtJsonDocPtr_t const& doc, sockaddr_storage const& peer);

  rtError open_rpc_listener();

  // command handlers
  // rtError on_search(rapidjson::Document const& doc, sockaddr* soc, socklen_t len);
  // rtError on_locate(rapidjson::Document const& doc, sockaddr* soc, socklen_t len);
  rtError on_open_session(rtJsonDocPtr_t const& doc, sockaddr_storage const& soc);
  rtError on_get_byname(rtJsonDocPtr_t const& doc, sockaddr_storage const& soc);
  rtError on_set_byname(rtJsonDocPtr_t const& doc, sockaddr_storage const& soc);
  rtError on_get_byindex(rtJsonDocPtr_t const& doc, sockaddr_storage const& soc);
  rtError on_set_byindex(rtJsonDocPtr_t const& doc, sockaddr_storage const& soc);

  rtError on_client_disconnect(connected_client& client);

  rtObjectRef get_object(rapidjson::Document const& doc) const;

private:
  struct object_reference
  {
    rtObjectRef object;
    std::vector<int> client_fds;
  };

  typedef std::map< std::string, object_reference > refmap_t;
  typedef std::map< std::string, command_handler_t > cmd_handler_map_t;
  typedef std::vector< connected_client > client_list_t;
  typedef std::map< std::string, std::shared_ptr<rtRpcTransport> > tport_map_t;

  sockaddr_storage        m_rpc_endpoint;
  int                     m_rpc_fd;
  pthread_t               m_thread;
  mutable pthread_cond_t          m_cond;
  mutable pthread_mutex_t         m_mutex;
  cmd_handler_map_t       m_command_handlers;

  refmap_t                m_objects;
  client_list_t           m_client_list;

  rtRemoteObjectResolver* m_resolver;
  tport_map_t             m_transports;
  int                     m_pipe_write;
  int                     m_pipe_read;
};
