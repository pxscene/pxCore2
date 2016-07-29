#include "rtRemoteIResolver.h"
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include <stdint.h>
#include <netinet/in.h>
#include <rtObject.h>
#include <rapidjson/document.h>

#include "rtRemoteTypes.h"
#include "rtSocketUtils.h"


class rtRemoteMulticastResolver : public rtRemoteIResolver
{
public:
  rtRemoteMulticastResolver(rtRemoteEnvironment* env);
  ~rtRemoteMulticastResolver();

public:
  virtual rtError open(sockaddr_storage const& rpc_endpoint) override;
  virtual rtError close() override;
  virtual rtError registerObject(std::string const& name, sockaddr_storage const& endpoint) override;
  virtual rtError locateObject(std::string const& name, sockaddr_storage& endpoint,
    uint32_t timeout) override;

private:
  using CommandHandler = rtError (rtRemoteMulticastResolver::*)(rtJsonDocPtr const&, sockaddr_storage const&);
  using HostedObjectsMap = std::map< std::string, sockaddr_storage >;
  using CommandHandlerMap = std::map< std::string, CommandHandler >;
  using RequestMap = std::map< rtCorrelationKey, rtJsonDocPtr >;

  void runListener();
  void doRead(int fd, rtSocketBuffer& buff);
  void doDispatch(char const* buff, int n, sockaddr_storage* peer);

  rtError init();
  rtError openUnicastSocket();
  rtError openMulticastSocket();

  // command handlers
  rtError onSearch(rtJsonDocPtr const& doc, sockaddr_storage const& soc);
  rtError onLocate(rtJsonDocPtr const& doc, sockaddr_storage const& soc);

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
  CommandHandlerMap m_command_handlers;
  std::string       m_rpc_addr;
  uint16_t          m_rpc_port;
  HostedObjectsMap  m_hosted_objects;
  RequestMap	      m_pending_searches;
  int		            m_shutdown_pipe[2];
  rtRemoteEnvironment* m_env;
};
