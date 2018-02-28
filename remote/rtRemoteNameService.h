#include "rtRemoteCorrelationKey.h"
#include "rtRemoteEndPoint.h"
#include "rtRemoteSocketUtils.h"
#include "rtRemoteEnvironment.h"

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include <stdint.h>
#include <rtObject.h>

class rtRemoteEnvironment;

class rtRemoteNameService
{
public:
  rtRemoteNameService(rtRemoteEnvironment* env);
  ~rtRemoteNameService();

public:
  rtError init();
  rtError close();

private:
  using CommandHandler = rtError (rtRemoteNameService::*)(rtRemoteMessagePtr const&, sockaddr_storage const&);
  using CommandHandlerMap = std::map< std::string, CommandHandler >;
  using RequestMap = std::map< rtRemoteCorrelationKey, rtRemoteMessagePtr >;
  using RegisteredObjectsMap = std::map< std::string, sockaddr_storage >;

  rtError onRegister(rtRemoteMessagePtr const& doc, sockaddr_storage const& soc);
  rtError onDeregister(rtRemoteMessagePtr const& doc, sockaddr_storage const& soc);
  rtError onUpdate(rtRemoteMessagePtr const& doc, sockaddr_storage const& soc);
  rtError onLookup(rtRemoteMessagePtr const& doc, sockaddr_storage const& soc);

  void runListener();
  void doRead(socket_t fd, rtRemoteSocketBuffer& buff);
  void doDispatch(char const* buff, int n, sockaddr_storage* peer);
  // rtError openDbConnection();
  rtError openNsSocket();

private:
  sockaddr_storage  m_ns_endpoint;
  socket_t          m_ns_fd;
  socklen_t         m_ns_len;
  
  int                  m_pid;
  CommandHandlerMap    m_command_handlers;
  RequestMap           m_pending_requests;
  RegisteredObjectsMap m_registered_objects;

  std::mutex                   m_mutex;
  std::unique_ptr<std::thread> m_read_thread;
  bool                         m_shutdown;
  rtRemoteEnvironment*         m_env;
};
