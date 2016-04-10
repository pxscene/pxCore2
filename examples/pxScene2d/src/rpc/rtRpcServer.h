#ifndef __RT_REMOTE_OBJECT_LOCATOR_H__
#define __RT_REMOTE_OBJECT_LOCATOR_H__

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include <stdint.h>
#include <netinet/in.h>
#include <rtObject.h>
#include <rapidjson/document.h>

#include "rtRpcTypes.h"
#include "rtRemoteObjectResolver.h"
#include "rtSocketUtils.h"

#ifdef __APPLE__
#define kDefaultMulticastInterface "en0"
#else
#define kDefaultMulticastInterface "eth0"
#endif
#define kDefaultIPv4MulticastAddress "224.10.0.12"
#define kDefaultIPv6MulticastAddress "ff05:0:0:0:0:0:0:201"
#define kDefaultMulticastPort 10004

class rtRpcClient;

class rtRpcServer
{
  friend class rtCommandDispatcher;

public:
  rtRpcServer();
  ~rtRpcServer();

public:
  rtError open(char const* dstaddr = nullptr, uint16_t dstport = 0,
    char const* srcaddr = nullptr);
  rtError start();

  rtError registerObject(std::string const& name, rtObjectRef const& obj);
  rtError findObject(std::string const& name, rtObjectRef& obj, uint32_t timeout = 1000);
  rtError removeStaleObjects(int* num_removed);

private:
  struct connected_client
  {
    sockaddr_storage    peer;
    int                 fd;
  };

  void runListener();
  void doAccept(int fd);

  rtError onIncomingMessage(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr_t const& msg);

  // command handlers
  rtError onOpenSession(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr_t const& doc);
  rtError onGet(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr_t const& doc);
  rtError onSet(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr_t const& doc);
  rtError onMethodCall(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr_t const& doc);
  rtError onKeepAlive(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr_t const& doc);
  rtError openRpcListener();

  rtObjectRef getObject(std::string const& id) const;

private:
  struct object_reference
  {
    rtObjectRef       object;
    std::vector<int>  client_fds;
    time_t            last_used;
    bool              owner_removed;
  };

  typedef std::map< std::string, object_reference > refmap_t;
  typedef std::map< std::string, rtRpcMessageHandler_t > command_handler_map;
  typedef std::map< std::string, std::shared_ptr<rtRpcClient> > tport_map_t;

  using client_list = std::vector< std::shared_ptr<rtRpcClient> >;

  sockaddr_storage              m_rpc_endpoint;
  int                           m_listen_fd;

  std::unique_ptr<std::thread>  m_thread;
  mutable std::mutex            m_mutex;
  command_handler_map		m_command_handlers;

//  refmap_t                      m_objects;

  rtRemoteObjectResolver*       m_resolver;
  tport_map_t                   m_transports;
  int                           m_pipe_write;
  int                           m_pipe_read;
  uint32_t                      m_keep_alive_interval;
  client_list			m_clients;
};

#endif
