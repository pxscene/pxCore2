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

#include "rtRemoteEndpointHandleStreamServer.h"
#include "rtRemoteEndpoint.h"
#include "rtRemoteTypes.h"
#include "rtRemoteUtils.h"
#include "rtRemoteSocketUtils.h"
#include "rtError.h"
#include <rtLog.h>

#include <sstream>
#include <string>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

rtRemoteEndpointHandleStreamServer::rtRemoteEndpointHandleStreamServer(rtRemoteEndpointPtr endpoint)
: rtRemoteIEndpointHandle(endpoint)
{
  memset(&m_socket, 0, sizeof(sockaddr_storage));
}

rtError
rtRemoteEndpointHandleStreamServer::open()
{
  if (m_addr == nullptr)
  {
    rtLogError("failed to open endpoint socket: endpoint address is null");
    return RT_FAIL;
  }
  rtError e = rtRemoteEndpointAddressToSocket(m_addr, m_socket);
  if (e != RT_OK)
  {
    rtLogError("failed to convert from endpoint address to sockaddr");
    return e;
  }
  
  // open socket
  m_fd = socket(m_socket.ss_family, SOCK_STREAM, 0);
  if (m_fd < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to create socket. %s", rtStrError(e));
    return e;
  }

  return RT_OK;
}

rtError
rtRemoteEndpointHandleStreamServer::close()
{
  if (m_fd != -1)
    ::close(m_fd);
  m_fd = -1;
  return RT_OK;
}

rtError
rtRemoteEndpointHandleStreamServer::doBind()
{
  int ret;
  socklen_t len;
  rtSocketGetLength(m_socket, &len);

  ret = bind(m_fd, reinterpret_cast<struct sockaddr*>(&m_socket), len);
  if (ret < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to bind socket. %s", rtStrError(e));
    return e;
  }

  rtGetSockName(m_fd, m_socket);
  rtLogInfo("local rpc listener on: %s", rtSocketToString(m_socket).c_str());

  return RT_OK;
}

rtError
rtRemoteEndpointHandleStreamServer::doListen()
{
  int ret;
  ret = listen(m_fd, 2);
  if (ret < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to put socket in listen mode. %s", rtStrError(e));
    return e;
  }
  return RT_OK;
}

rtError
rtRemoteEndpointHandleStreamServer::doAccept(int& new_fd, rtRemoteEndpointPtr& remote_addr)
{
  sockaddr_storage remote_endpoint;
  memset(&remote_endpoint, 0, sizeof(remote_endpoint));

  socklen_t len = sizeof(sockaddr_storage);

  new_fd = accept(m_fd, reinterpret_cast<struct sockaddr*>(&remote_endpoint), &len);

  if (new_fd == -1)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogWarn("error accepting new tcp connect. %s", rtStrError(e));
    return RT_FAIL;
  }
  rtLogInfo("new connection from %s with fd:%d", rtSocketToString(remote_endpoint).c_str(), new_fd);
  return rtRemoteSocketToEndpointAddress(remote_endpoint, rtConnType::STREAM, remote_addr);
}
