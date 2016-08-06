#include "rtRemoteIResolver.h"
#include "rtRemoteTypes.h"
#include "rtRemoteCorrelationKey.h"
#include "rtRemoteSocketUtils.h"

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include <stdint.h>
#include <netinet/in.h>
#include <rtObject.h>

class rtRemoteEnvironment;

class rtRemoteNsResolver : public rtRemoteIResolver
{
public:
  rtRemoteNsResolver(rtRemoteEnvironment* env);
  ~rtRemoteNsResolver();

public:
  virtual rtError open(sockaddr_storage const& rpc_endpoint) override;
  virtual rtError close() override;
  virtual rtError registerObject(std::string const& name, sockaddr_storage const& endpoint) override;
  virtual rtError locateObject(std::string const& name, sockaddr_storage& endpoint,
    uint32_t timeout) override;
  rtError registerObject(std::string const& name, sockaddr_storage const& endpoint, uint32_t timeout);

private:
  using CommandHandler = rtError (rtRemoteNsResolver::*)(rtRemoteMessagePtr const&, sockaddr_storage const&);
  using HostedObjectsMap = std::map< std::string, sockaddr_storage >;
  using CommandHandlerMap = std::map< std::string, CommandHandler >;
  using RequestMap = std::map< rtRemoteCorrelationKey, rtRemoteMessagePtr >;

  void runListener();
  void doRead(int fd, rtSocketBuffer& buff);
  void doDispatch(char const* buff, int n, sockaddr_storage* peer);

  rtError init();
  rtError openSocket();

  // command handlers
  rtError onLocate(rtRemoteMessagePtr const& doc, sockaddr_storage const& soc);

private:

  sockaddr_storage  m_static_endpoint;
  int               m_static_fd;
  socklen_t         m_static_len;

  std::unique_ptr<std::thread> m_read_thread;
  std::condition_variable m_cond;
  std::mutex        m_mutex;
  pid_t             m_pid;
  CommandHandlerMap m_command_handlers;
  std::string       m_rpc_addr;
  uint16_t          m_rpc_port;
  HostedObjectsMap  m_hosted_objects;
  RequestMap	    m_pending_searches;
  int		        m_shutdown_pipe[2];

  sockaddr_storage  m_ns_dest;
  rtRemoteEnvironment*  m_env;

  // endpointMapper;
  // 
};
