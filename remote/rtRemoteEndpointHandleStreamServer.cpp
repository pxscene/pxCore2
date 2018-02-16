#include "rtRemoteEndpointHandleStreamServer.h"
#include "rtRemoteEndPoint.h"
#include "rtRemoteTypes.h"
#include "rtRemoteUtils.h"
#include "rtRemoteSocketUtils.h"
#include "rtError.h"
#include <rtLog.h>

#include <sstream>
#include <string>
#include <errno.h>
#include <stdlib.h>

rtRemoteEndpointHandleStreamServer::rtRemoteEndpointHandleStreamServer(rtRemoteEndPointPtr endpoint)
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
  if (NET_FAILED(m_fd))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to create socket. %s", rtStrError(e));
    return e;
  }

  return RT_OK;
}

rtError
rtRemoteEndpointHandleStreamServer::close()
{
  if (m_fd != kInvalidSocket)
    rtCloseSocket(m_fd);
  m_fd = kInvalidSocket;
  return RT_OK;
}

rtError
rtRemoteEndpointHandleStreamServer::doBind()
{
  int ret;
  socklen_t len;
  rtSocketGetLength(m_socket, &len);

  ret = bind(m_fd, reinterpret_cast<struct sockaddr*>(&m_socket), len);
  if (NET_FAILED(ret))
  {
    rtError e = rtErrorFromErrno(net_errno());
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
  if (NET_FAILED(ret))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to put socket in listen mode. %s", rtStrError(e));
    return e;
  }
  return RT_OK;
}

rtError
rtRemoteEndpointHandleStreamServer::doAccept(int& new_fd, rtRemoteEndPointPtr& remote_addr)
{
  sockaddr_storage remote_endpoint;
  memset(&remote_endpoint, 0, sizeof(remote_endpoint));

  socklen_t len = sizeof(sockaddr_storage);

  new_fd = accept(m_fd, reinterpret_cast<struct sockaddr*>(&remote_endpoint), &len);

  if (NET_FAILED(new_fd))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogWarn("error accepting new tcp connect. %s", rtStrError(e));
    return RT_FAIL;
  }
  rtLogInfo("new connection from %s with fd:%d", rtSocketToString(remote_endpoint).c_str(), new_fd);
  return rtRemoteSocketToEndpointAddress(remote_endpoint, rtConnType::STREAM, remote_addr);
}