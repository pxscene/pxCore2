/* 
 * Copyright [2017] [Comcast, Corp.]
 *
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

#include <string.h>
#include <unistd.h>

static const rtSocketHandle rtSocketInvalidSocketHandle = -1;

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

rtSocket::rtSocket()
  : m_handle(rtSocketInvalidSocketHandle)
  , m_localEndPoint(rtIpAddress::fromString(NULL), 0)
  , m_remoteEndPoint(rtIpAddress::fromString(NULL), 0)
{
}

rtSocket::~rtSocket()
{
  this->close();
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
      memcpy(&m_localEndPoint.m_addr.m_addr, &endpoint.m_addr.m_addr, sizeof(sockaddr_storage));
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

  return e;
}

rtSocket
rtSocket::accept()
{
  rtSocket soc;

  socklen_t len = sizeof(sockaddr_storage);
  int ret = ::accept(m_handle, reinterpret_cast<sockaddr *>(&soc.m_remoteEndPoint.m_addr.m_addr), &len);
  if (ret != -1)
  {
    soc.m_handle = ret;

    // get local interface in use length of socket structure for local interface should
    // match that of remote 
    rtError e = RT_OK;
    
    e = getBindInterface(soc.m_handle, soc.m_localEndPoint, len, true);
    if (e != RT_OK)
      rtLogWarn("failed to get locally bound interface. %s", rtStrError(e));
  }
  else
  {
    rtErrorSetLastError(rtErrorFromErrno(errno));
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
