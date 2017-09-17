/* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "rtSocket.h"
#include "rtLog.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static const rtSocketHandle rtSocketInvalidSocketHandle = -1;

#define RT_ERROR_CODE(ERR) ((ERR) & 0x0000ffff)

rtSocket::rtSocket()
  : m_handle(rtSocketInvalidSocketHandle)
  , m_localEndPoint(rtIpAddress::fromString(NULL), 0)
  , m_remoteEndPoint(rtIpAddress::fromString(NULL), 0)
  , m_passiveListen(false)
{
}

rtSocket::rtSocket(rtSocket const& /*rhs*/)
  : m_handle(rtSocketInvalidSocketHandle)
  , m_localEndPoint(rtIpAddress::fromString(NULL), 0)
  , m_remoteEndPoint(rtIpAddress::fromString(NULL), 0)
  , m_passiveListen(false)
{
  RT_ASSERT(false);
}

rtSocket::~rtSocket()
{
  this->close();
}

rtSocketHandle
rtSocket::handle() const
{
  return m_handle;
}

rtError
rtSocket::onReadyRead(void* argp)
{
  rtLogDebug("no handler for onReadyRead: %p", argp);
  return RT_OK;
}

rtError
rtSocket::onReadyWrite(void* argp)
{
  rtLogDebug("no handler for onReadyWrite: %p", argp);
  return RT_OK;
}

rtError
rtSocket::onReadyAccept(void* argp)
{
  rtLogDebug("no handler for onReadyAccept: %p", argp);
  return RT_OK;
}

rtError
rtSocket::onReadyConnect(void* argp)
{
  rtLogDebug("no handler for onReadyConnect: %p", argp);
  return RT_OK;
}

rtError
rtSocket::onError(rtError e, void* argp)
{
  rtLogDebug("no handler for onError: %s, %p", rtStrError(e), argp);
  return RT_OK;
}

rtSocket const&
rtSocket::operator = (rtSocket const& /*rhs*/)
{
  RT_ASSERT(false);
  return *this;
}


rtError
rtSocket::close()
{
  rtError e = RT_OK;

  if (m_handle != rtSocketInvalidSocketHandle)
  {
    int ret = ::close(m_handle);
    if (ret != 0)
      e = rtErrorFromErrno(ret);
    m_handle = rtSocketInvalidSocketHandle;
  }

  return e;
}

int
rtSocket::send(void* buff, int n)
{
  if (m_handle == rtSocketInvalidSocketHandle)
  {
    rtErrorSetLastError(RT_ERROR_INVALID_OPERATION);
    return -1;
  }

  ssize_t ret = ::send(m_handle, buff, n, MSG_NOSIGNAL);
  if (ret == -1)
  {
    rtErrorSetLastError(rtErrorFromErrno(ret));
    return -1;
  }
  else
  {
    rtErrorSetLastError(RT_OK);
  }

  return static_cast<int>(ret);
}

int
rtSocket::recv(void* buff, int n)
{
  return static_cast<int>(::recv(m_handle, buff, n, 0));
}


rtError
rtSocket::createSocket(int family)
{
  rtError e = RT_OK;
  m_handle = ::socket(family, SOCK_STREAM, 0);
  if (m_handle == -1)
    e = rtErrorFromErrno(errno);
  return e;
}

rtError
rtSocket::connect(char const* addr, uint16_t port)
{
  rtIpAddress ip = rtIpAddress::fromString(addr);
  rtIpEndPoint endpoint(ip, port);
  return this->connect(endpoint);
}

rtError
rtSocket::connect(rtIpEndPoint const& endpoint)
{
  rtError e = RT_OK;
  rtIpAddress addr = endpoint.address();

  if (m_handle == rtSocketInvalidSocketHandle)
    e = createSocket(addr.m_addr.ss_family);

  if (e == RT_OK)
  {
    int ret = ::connect(m_handle, reinterpret_cast<sockaddr *>(&addr.m_addr), addr.length());
    if (ret == -1)
      e = rtErrorFromErrno(errno);
  }

  if (e == RT_OK)
  {
    getBindInterface(m_handle, m_remoteEndPoint, addr.length(), false);
    getBindInterface(m_handle, m_localEndPoint, addr.length(), true);
  }

  return e;
}

rtError
rtSocket::bind(rtIpEndPoint const& endpoint)
{
  rtError e = RT_OK;
  rtIpAddress addr = endpoint.address();

  if (m_handle == rtSocketInvalidSocketHandle)
    e = createSocket(addr.m_addr.ss_family);

  if (e == RT_OK)
  {
    int ret = ::bind(m_handle, reinterpret_cast<sockaddr *>(&addr.m_addr), addr.length());
    if (ret == 0)
      getBindInterface(m_handle, m_localEndPoint, addr.length(), true);
    else if (ret == -1)
      e = rtErrorFromErrno(errno);
  }

  return e;
}

rtError
rtSocket::listen()
{
  rtError e = RT_OK;

  int ret = ::listen(m_handle, 3);
  if (ret != 0)
    e = rtErrorFromErrno(errno);
  else
    m_passiveListen = true;

  return e;
}

bool
rtSocket::blocking() const
{
  int flags = fcntl(m_handle, F_GETFL, 0);
  return (flags & O_NONBLOCK) == O_NONBLOCK;
}

rtError
rtSocket::setBlocking(bool b)
{
  rtError e = RT_OK;

  int flags = fcntl(m_handle, F_GETFL, 0);
  if (b)
    flags |= O_NONBLOCK;
  else
    flags &= ~O_NONBLOCK;
  flags = fcntl(m_handle, F_SETFL, flags);

  if (flags == -1)
    e = rtErrorFromErrno(errno);

  return  e;
}

rtSocket*
rtSocket::accept()
{
  rtSocket* soc = NULL;

  sockaddr_storage remote_endpoint;
  memset(&remote_endpoint, 0, sizeof(remote_endpoint));

  socklen_t len = sizeof(sockaddr_storage);
//  int ret = ::accept(m_handle, reinterpret_cast<sockaddr *>(&soc->m_remoteEndPoint.m_addr.m_addr), &len);
  int ret = ::accept(m_handle, reinterpret_cast<sockaddr *>(&remote_endpoint), &len);
  if (ret != -1)
  {
    rtLogInfo("accepted ok: %d", ret);

    soc = new rtSocket();
    soc->m_handle = ret;

    // get local interface in use length of socket structure for local interface should
    // match that of remote 
    rtError e = RT_OK;
    
    e = getBindInterface(soc->m_handle, soc->m_localEndPoint, len, true);
    if (e != RT_OK)
      rtLogWarn("failed to get locally bound interface. %s", rtStrError(e));

    memset(&soc->m_remoteEndPoint.m_addr.m_addr, 0, sizeof(soc->m_remoteEndPoint.m_addr.m_addr));
    memcpy(&soc->m_remoteEndPoint.m_addr.m_addr, &remote_endpoint, sizeof(sockaddr_storage));
  }
  else
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogWarn("failed to accept new connection: %s", rtStrError(e));
    rtErrorSetLastError(e);
  }

  return soc;
}

rtIpEndPoint
rtSocket::localEndPoint() const
{
  return m_localEndPoint;
}

rtIpEndPoint
rtSocket::remoteEndPoint() const
{
  return m_remoteEndPoint;
}

rtError
rtSocket::getBindInterface(int fd, rtIpEndPoint& endpoint, socklen_t len, bool local)
{
  sockaddr_storage& addr = endpoint.m_addr.m_addr;
  memset(&addr, 0, sizeof(sockaddr_storage));

  int ret = 0;
  if (local)
    ret = getsockname(fd, reinterpret_cast<sockaddr *>(&addr), &len);
  else
    ret = getpeername(fd, reinterpret_cast<sockaddr *>(&addr), &len);

  if (ret == -1)
  {
    rtError err = rtErrorFromErrno(errno);
    return err;
  }

  memcpy(&endpoint, &addr, sizeof(sockaddr_storage));
  return RT_OK;
}


rtTcpListener::rtTcpListener(rtIpEndPoint const& bindEndpoint)
{
  m_localEndPoint = bindEndpoint;
}

rtTcpListener::~rtTcpListener()
{
  close();
}

rtError
rtTcpListener::start(bool blocking)
{
  rtError e = RT_OK;
  e = this->bind(m_localEndPoint);
  if (e != RT_OK)
  {
    rtLogWarn("failed to bind socket to %s. %s", m_localEndPoint.toString().c_str(), rtStrError(e));
    return e;
  }

  if (e == RT_OK)
  {
    e = this->listen();
    rtLogDebug("listener returned:%s", rtStrError(e));
  }

  if (e == RT_OK)
  {
    e = this->setBlocking(blocking);
    rtLogDebug("set to blocking (%d) returned:%s", blocking, rtStrError(e));
  }

  return e;
}

rtTcpClient*
rtTcpListener::acceptClient()
{
  rtTcpClient* client = NULL;
  #if 0
  rtSocket* soc = m_soc->accept();
  if (soc)
    client = new rtTcpClient(soc);
  #endif
  return client;
}

rtTcpClient::~rtTcpClient()
{
  close();
}
