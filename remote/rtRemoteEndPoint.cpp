#include "rtRemoteEndPoint.h"
#include "rtLog.h"
#include "rtRemoteSocketUtils.h"
#include <string.h>
#include <arpa/inet.h>

rtRemoteEndPoint*
rtRemoteEndPoint::fromString(std::string const& s)
{
  rtRemoteEndPoint* e = nullptr;
  if (strncmp(s.c_str(), "unix", 4) == 0)
  {
    e = rtRemoteFileEndPoint::fromString(s);
  }
  else if ((strncmp(s.c_str(), "tcp", 4) == 0) || 
    (strncmp(s.c_str(), "udp", 3) == 0) || (strncmp(s.c_str(), "mcast", 4) == 0))
  {
    e = rtRemoteIPEndPoint::fromString(s);
  }

  return e;
}

sockaddr_storage
rtRemoteFileEndPoint::toSockAddr() const
{
  sockaddr_storage s;
  memset(&s, 0, sizeof(s));
  rtParseAddress(s, m_path.c_str(), 0, nullptr);
  return s;
}

rtRemoteFileEndPoint*
rtRemoteFileEndPoint::fromSockAddr(sockaddr_storage const& s)
{
  void* addr = nullptr;
  rtGetInetAddr(s, &addr);

  return new rtRemoteFileEndPoint("unix", reinterpret_cast<char const *>(addr));
}

sockaddr_storage
rtRemoteIPEndPoint::toSockAddr() const
{
  sockaddr_storage s;
  memset(&s, 0, sizeof(s));
  rtParseAddress(s, m_host.c_str(), m_port, nullptr);
  return s;
}

rtRemoteIPEndPoint*
rtRemoteIPEndPoint::fromSockAddr(std::string const& scheme, sockaddr_storage const& ss)
{
  socklen_t len;
  rtSocketGetLength(ss, &len);

  uint16_t port;
  rtGetPort(ss, &port);

  void* addr = nullptr;
  rtGetInetAddr(ss, &addr);

  char buff[256];
  memset(buff, 0, sizeof(buff));
  char const* p = inet_ntop(ss.ss_family, addr, buff, len);

  return new rtRemoteIPEndPoint(scheme, p, port);
}

rtRemoteIPEndPoint*
rtRemoteIPEndPoint::fromString(std::string const& s)
{
  // <scheme>://<ip>:port
  rtRemoteIPEndPoint* e(new rtRemoteIPEndPoint());

  std::string::size_type begin, end;
  
  begin = 0;
  end = s.find("://");
  if (end == std::string::npos)
    return e;

  e->m_scheme = s.substr(begin, end);

  begin = end + 3;
  end = s.find(':', begin);
  if (end == std::string::npos)
    return e;
  e->m_host = s.substr(begin, (end - begin));
  e->m_port = std::stoi(s.substr(end + 1));
  return e;
}

rtRemoteFileEndPoint*
rtRemoteFileEndPoint::fromString(std::string const& s)
{
  rtRemoteFileEndPoint* e(new rtRemoteFileEndPoint());

  std::string::size_type begin, end;
  begin = 0;
  end = s.find("://");
  if (end == std::string::npos)
    return e;
  e->m_scheme = s.substr(begin, end);
  e->m_path = s.substr(end + 3);
  return e;
}
