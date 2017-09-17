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
#include "rtIpAddress.h"
#include "rtError.h"
#include "rtLog.h"

#include <sstream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

inline sockaddr_in const*
as4(sockaddr_storage const& ss)
{
  return reinterpret_cast<sockaddr_in const*>(&ss);
}

inline sockaddr_in6 const*
as6(sockaddr_storage const& ss)
{
  return reinterpret_cast<sockaddr_in6 const *>(&ss);
}

static rtError
rtParseAddress(sockaddr_storage& ss, char const* addr)
{
  rtError e = RT_OK;
  sockaddr_in* v4 = reinterpret_cast<sockaddr_in *>(&ss);

  int ret = inet_pton(AF_INET, addr, &v4->sin_addr);
  if (ret == 1)
  {
    #ifndef __linux__
    v4->sin_len = sizeof(sockaddr_in);
    #endif
    v4->sin_family = AF_INET;
    v4->sin_port = 0;
    ss.ss_family = AF_INET;
  }
  else if (ret == 0)
  {
    sockaddr_in6* v6 = reinterpret_cast<sockaddr_in6 *>(&ss);
    ret = inet_pton(AF_INET6, addr, &v6->sin6_addr);
    if (ret == 1)
    {
      v6->sin6_family = AF_INET6;
      v6->sin6_port = 0;
      #if 0
      #ifndef __linux__
      v6->sin6_len = sizeof(sockaddr_in6);
      #endif
      #endif
    }
    else if (ret == -1)
    {
      e = rtErrorFromErrno(errno);
    }
  }

  if (e == RT_OK && ret == -1)
    e = rtErrorFromErrno(errno);

  return e;
}

rtIpAddress::rtIpAddress()
{
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.ss_family = AF_INET;
}

inline void const*
addr(sockaddr_storage const& ss)
{
  if (ss.ss_family == AF_INET)
    return &(as4(ss)->sin_addr);
  if (ss.ss_family == AF_INET6)
    return &(as6(ss)->sin6_addr);
  RT_ASSERT(false);
  return NULL;
}

std::string
rtIpAddress::toString() const
{
  std::string s;

  char buff[128];
  memset(buff, 0, sizeof(buff));
  inet_ntop(m_addr.ss_family, addr(m_addr), buff, sizeof(buff));
  s = buff;

  return s;
}

bool
rtIpAddress::operator == (rtIpAddress const& rhs) const
{
  if (this == &rhs)
    return true;

  // TODO: hack for now
  return toString() == rhs.toString();
}

socklen_t
rtIpAddress::length() const
{
  socklen_t len = 0;
  if (m_addr.ss_family == AF_INET)
    len = sizeof(sockaddr_in);
  if (m_addr.ss_family == AF_INET6)
    len = sizeof(sockaddr_in6);
  return len;
}

rtIpAddress
rtIpAddress::fromString(char const* s)
{
  if (s == NULL || strlen(s) == 0)
    return rtIpAddress();

  rtIpAddress addr;
  rtError e = rtParseAddress(addr.m_addr, s);
  rtErrorSetLastError(e);
  return addr;
}

rtIpEndPoint::rtIpEndPoint(rtIpAddress const& addr, uint16_t port)
  : m_addr(addr)
{
  if (m_addr.m_addr.ss_family == AF_INET)
    reinterpret_cast<sockaddr_in *>(&m_addr.m_addr)->sin_port = htons(port);
  if (m_addr.m_addr.ss_family == AF_INET6)
    reinterpret_cast<sockaddr_in6 *>(&m_addr.m_addr)->sin6_port = htons(port);
}

rtIpAddress
rtIpEndPoint::address() const
{
  return m_addr;
}

uint16_t
rtIpEndPoint::port() const
{
  uint16_t port = 0;
  if (m_addr.m_addr.ss_family == AF_INET)
    port = ntohs(reinterpret_cast<sockaddr_in const *>(&m_addr.m_addr)->sin_port);
  if (m_addr.m_addr.ss_family == AF_INET6)
    port = htons(reinterpret_cast<sockaddr_in6 const *>(&m_addr.m_addr)->sin6_port);
  return port;
}

std::string
rtIpEndPoint::toString() const
{
  std::stringstream buff;
  buff << m_addr.toString();
  buff << ':';
  buff << port();
  return buff.str();
}

rtIpEndPoint
rtIpEndPoint::fromString(char const* s)
{
  char buff[256];
  memset(buff, 0, sizeof(buff));

  char const* p = strchr(s, ':');
  strncpy(buff, s, (p-s));

  rtIpAddress addr = rtIpAddress::fromString(buff);
  if (rtErrorGetLastError() != RT_OK)
    return rtIpEndPoint::invalid();

  long int port = strtol(p + 1, NULL, 10);
  if (errno == ERANGE)
  {
  }

  // TODO: check uint16_t bounds

  return rtIpEndPoint(addr, static_cast<uint16_t>(port));
}

rtIpEndPoint const&
rtIpEndPoint::invalid()
{
  static rtIpEndPoint _invalid(rtIpAddress::fromString("1.1.1.1"), 11111);
  return _invalid;
}

bool
rtIpEndPoint::operator == (rtIpEndPoint const& rhs) const
{
  if (this == &rhs)
    return true;
  return this->address() == rhs.address() &&
    this->port() == rhs.port();
}
