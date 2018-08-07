/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "rtRemoteCorrelationKey.h"
#include "rtRemoteEndpoint.h"
#include "rtRemoteSocketUtils.h"
#include "rtRemoteEnvironment.h"

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include <stdint.h>
#include <netinet/in.h>
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
  void doRead(int fd, rtRemoteSocketBuffer& buff);
  void doDispatch(char const* buff, int n, sockaddr_storage* peer);
  // rtError openDbConnection();
  rtError openNsSocket();

private:
  sockaddr_storage  m_ns_endpoint;
  int               m_ns_fd;
  socklen_t         m_ns_len;
  
  pid_t                        m_pid;
  CommandHandlerMap    m_command_handlers;
  RequestMap           m_pending_requests;
  RegisteredObjectsMap m_registered_objects;

  std::mutex                   m_mutex;
  std::unique_ptr<std::thread> m_read_thread;
  int		                       m_shutdown_pipe[2];
  rtRemoteEnvironment*         m_env;
};
