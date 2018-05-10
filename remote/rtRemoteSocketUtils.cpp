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

#include "rtRemoteSocketUtils.h"

#include <cstdio>
#include <sstream>

#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include <rtLog.h>

#include <rapidjson/memorystream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#ifndef RT_REMOTE_LOOPBACK_ONLY
static rtError
rtFindFirstInetInterface(char* name, size_t len)
{
  rtError e = RT_FAIL;
  ifaddrs* ifaddr = NULL;
  int ret = getifaddrs(&ifaddr);
  if (ret == -1)
  {
    e = rtErrorFromErrno(errno);
    rtLogError("failed to get list of interfaces: %s", rtStrError(e));
    return e;
  }

  for (ifaddrs* i = ifaddr; i != nullptr; i = i->ifa_next)
  {
    if (i->ifa_addr == nullptr)
      continue;
    if (i->ifa_addr->sa_family != AF_INET && i->ifa_addr->sa_family != AF_INET6)
      continue;
    if (strcmp(i->ifa_name, "lo") == 0)
      continue;

    strncpy(name, i->ifa_name, len);
    e = RT_OK;
    break;
  }

  if (ifaddr)
    freeifaddrs(ifaddr);

  return e;
}
#endif


rtError
rtParseAddress(sockaddr_storage& ss, char const* addr, uint16_t port, uint32_t* index)
{
  int ret = 0;

  if (index != nullptr)
    *index = -1;

  if (addr[0] == '/')
  {
    sockaddr_un *unAddr = reinterpret_cast<sockaddr_un*>(&ss);
    unAddr->sun_family = AF_UNIX;
    strncpy(unAddr->sun_path, addr, UNIX_PATH_MAX);
    return RT_OK;
  }

  sockaddr_in* v4 = reinterpret_cast<sockaddr_in *>(&ss);
  ret = inet_pton(AF_INET, addr, &v4->sin_addr);

  if (ret == 1)
  {
    #ifndef __linux__
    v4->sin_len = sizeof(sockaddr_in);
    #endif

    v4->sin_family = AF_INET;
    v4->sin_port = htons(port);
    ss.ss_family = AF_INET;
    return RT_OK;
  }
  else if (ret == 0)
  {
    sockaddr_in6* v6 = reinterpret_cast<sockaddr_in6 *>(&ss);
    ret = inet_pton(AF_INET6, addr, &v6->sin6_addr);
    if (ret == 0)
    {
      // try hostname
      rtError err = rtGetInterfaceAddress(addr, ss);
      if (err != RT_OK)
        return err;

      if (index != nullptr)
        *index = if_nametoindex(addr);

      if (ss.ss_family == AF_INET)
      {
        v4->sin_family = AF_INET;
        v4->sin_port = htons(port);
      }
      if (ss.ss_family == AF_INET6)
      {
        v6->sin6_family = AF_INET6;
        v6->sin6_port = htons(port);
      }
    }
    else if (ret == 1)
    {
      // it's a numeric address
      v6->sin6_family = AF_INET6;
      v6->sin6_port = htons(port);
    }
  }
  else
  {
    return rtErrorFromErrno(errno);
  }
  return RT_OK;
}

rtError
rtSocketGetLength(sockaddr_storage const& ss, socklen_t* len)
{
  if (!len)
    return RT_ERROR_INVALID_ARG;

  if (ss.ss_family == AF_INET)
    *len = sizeof(sockaddr_in);
  else if (ss.ss_family == AF_INET6)
    *len = sizeof(sockaddr_in6);
  else if (ss.ss_family == AF_UNIX)
    *len = sizeof(sockaddr_un);
  else
    *len = sizeof(sockaddr_storage);

  return RT_OK;
}

rtError
rtGetInterfaceAddress(char const* name, sockaddr_storage& ss)
{
  rtError error = RT_FAIL;
  ifaddrs* ifaddr = NULL;

  int ret = getifaddrs(&ifaddr);

  if (ret == -1)
  {
    error = rtErrorFromErrno(errno);
    rtLogError("failed to get list of interfaces. %s", rtStrError(error));
    return error;
  }

  for (ifaddrs* i = ifaddr; i != NULL; i = i->ifa_next)
  {
    if (i->ifa_addr == NULL)
      continue;

    if (strcmp(name, i->ifa_name) != 0)
      continue;

    if (i->ifa_addr->sa_family != AF_INET && i->ifa_addr->sa_family != AF_INET6)
      continue;

    ss.ss_family = i->ifa_addr->sa_family;

    socklen_t len;
    rtSocketGetLength(ss, &len);

    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    ret = getnameinfo(i->ifa_addr, len, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST);
    if (ret != 0)
    {
      // TODO: add error class for gai errors
      rtLogError("failed to get address for %s. %s", name, gai_strerror(ret));
      error = RT_FAIL;
      goto out;
    }
    else
    {
      void* addr = NULL;
      rtGetInetAddr(ss, &addr);

      ret = inet_pton(ss.ss_family, host, addr);
      if (ret != 1)
      {
        int err = errno;
        rtLogError("failed to parse: %s as valid ipv4 address", host);
        error = rtErrorFromErrno(err);
      }
      error = RT_OK;
      goto out;
    }
  }

out:
  if (ifaddr)
    freeifaddrs(ifaddr);

  return error;
}

rtError
rtGetInetAddr(sockaddr_storage const& ss, void** addr)
{
  sockaddr_in const* v4 = reinterpret_cast<sockaddr_in const*>(&ss);
  sockaddr_in6 const* v6 = reinterpret_cast<sockaddr_in6 const*>(&ss);
  sockaddr_un const* un =  reinterpret_cast<sockaddr_un const*>(&ss);

  if (ss.ss_family == AF_UNIX)
  {
    void const* p = reinterpret_cast<void const *>(&(un->sun_path));
    *addr = const_cast<void *>(p);
    return RT_OK;
  }

  void const* p = (ss.ss_family == AF_INET)
    ? reinterpret_cast<void const *>(&(v4->sin_addr))
    : reinterpret_cast<void const *>(&(v6->sin6_addr));

  *addr = const_cast<void *>(p);

  return RT_OK;
}

rtError
rtGetPort(sockaddr_storage const& ss, uint16_t* port)
{
  if (ss.ss_family == AF_UNIX)
  {
    *port = 0;
    return RT_OK;
  }
  sockaddr_in const* v4 = reinterpret_cast<sockaddr_in const *>(&ss);
  sockaddr_in6 const* v6 = reinterpret_cast<sockaddr_in6 const *>(&ss);
  *port = ntohs((ss.ss_family == AF_INET) ? v4->sin_port : v6->sin6_port);
  return RT_OK;
}

rtError
rtPushFd(fd_set* fds, int fd, int* maxFd)
{
  if (fd != -1)
  {
    FD_SET(fd, fds);
    if (maxFd && fd > *maxFd)
      *maxFd = fd;
  }
  return RT_OK;
}

rtError
rtReadUntil(int fd, char* buff, int n)
{
  ssize_t bytesRead = 0;
  ssize_t bytesToRead = n;

  while (bytesRead < bytesToRead)
  {
    ssize_t n = read(fd, buff + bytesRead, (bytesToRead - bytesRead));
    if (n == 0)
      return rtErrorFromErrno(ENOTCONN);

    if (n == -1)
    {
      if (errno == EINTR)
        continue;
      rtError e = rtErrorFromErrno(errno);
      rtLogError("failed to read from fd %d. %s", fd, rtStrError(e));
      return e;
    }

    bytesRead += n;
  }
  return RT_OK;
}

std::string
rtSocketToString(sockaddr_storage const& ss)
{
  // TODO make this more effecient
  void* addr = NULL;
  rtGetInetAddr(ss, &addr);

  uint16_t port;
  rtGetPort(ss, &port);

  char addrBuff[128];
  memset(addrBuff, 0, sizeof(addrBuff));
  if (ss.ss_family == AF_UNIX)
  {
    strncpy(addrBuff, reinterpret_cast<char const *>(addr), sizeof(addrBuff) -1);
    port = 0;
  }
  else
  {
    inet_ntop(ss.ss_family, addr, addrBuff, sizeof(addrBuff));
  }

  std::stringstream buff;
  buff << (ss.ss_family == AF_UNIX ? "unix" : "inet");
  buff << ':';
  buff << addrBuff;
  buff << ':';
  buff << port;
  return buff.str();
}

rtError
rtSendDocument(rapidjson::Document const& doc, int fd, sockaddr_storage const* dest)
{
  rapidjson::StringBuffer buff;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buff);
  doc.Accept(writer);

  #ifdef RT_RPC_DEBUG
  sockaddr_storage remoteEndpoint;
  memset(&remoteEndpoint, 0, sizeof(sockaddr_storage));
  if (dest)
    remoteEndpoint = *dest;
  else
    rtGetPeerName(fd, remoteEndpoint);

  char const* verb = (dest != NULL ? "sendto" : "send");
  rtLogDebug("%s [%d/%s] (%d):\n***OUT***\t\"%.*s\"\n",
    verb,
    fd,
    rtSocketToString(remoteEndpoint).c_str(),
    static_cast<int>(buff.GetSize()),
    static_cast<int>(buff.GetSize()),
    buff.GetString());
  #endif

  if (dest)
  {
    socklen_t len;
    rtSocketGetLength(*dest, &len);

    int flags = 0;
    #ifndef __APPLE__
    flags = MSG_NOSIGNAL;
    #endif

    if (sendto(fd, buff.GetString(), buff.GetSize(), flags,
          reinterpret_cast<sockaddr const *>(dest), len) < 0)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogError("sendto failed. %s. dest:%s family:%d", rtStrError(e), rtSocketToString(*dest).c_str(),
        dest->ss_family);
      return e;
    }
  }
  else
  {
    // send length first
    int n = buff.GetSize();
    n = htonl(n);

    struct msghdr msg;
    struct iovec iov[2];
    memset (&msg, '\0', sizeof (msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;
    iov[0].iov_base = &n;
    iov[0].iov_len = sizeof(n);
    iov[1].iov_base = const_cast<char*>(buff.GetString());
    iov[1].iov_len = buff.GetSize();

    int flags = 0;
    #ifndef __APPLE__
    flags = MSG_NOSIGNAL;
    #endif

    while (sendmsg (fd, &msg, flags) < 0)
    {
      if (errno == EINTR)
        continue;
      rtError e = rtErrorFromErrno(errno);
      rtLogError("failed to send message. %s", rtStrError(e));
      return e;
    }
  }

  return RT_OK;
}

rtError
rtReadMessage(int fd, rtRemoteSocketBuffer& buff, rtRemoteMessagePtr& doc)
{
  rtError err = RT_OK;

  int n = 0;
  int capacity = static_cast<int>(buff.capacity());

  err = rtReadUntil(fd, reinterpret_cast<char *>(&n), 4);
  if (err != RT_OK)
    return err;

  n = ntohl(n);

  if (n > capacity)
  {
    rtLogWarn("buffer capacity %d not big enough for message size: %d", capacity, n);
    // TODO: should drain, and discard message
    RT_ASSERT(false);
    return RT_FAIL;
  }

  buff.resize(n + 1);
  buff[n] = '\0';

  err = rtReadUntil(fd, &buff[0], n);
  if (err != RT_OK)
  {
    rtLogError("failed to read payload message of length %d from socket", n);
    return err;
  }

  #ifdef RT_RPC_DEBUG
  rtLogDebug("read (%d):\n***IN***\t\"%.*s\"\n", static_cast<int>(buff.size()), static_cast<int>(buff.size()), &buff[0]);
  #endif

  return rtParseMessage(&buff[0], n, doc);
}

rtError
rtParseMessage(char const* buff, int n, rtRemoteMessagePtr& doc)
{
  RT_ASSERT(buff != nullptr);
  RT_ASSERT(n > 0);

  if (!buff)
    return RT_FAIL;

  doc.reset(new rapidjson::Document());

  rapidjson::MemoryStream stream(buff, n);
  if (doc->ParseStream<rapidjson::kParseDefaultFlags>(stream).HasParseError())
  {
    int begin = doc->GetErrorOffset() - 16;
    if (begin < 0)
      begin = 0;
    int end = begin + 64;
    if (end > n)
      end = n;
    int length = (end - begin);

    rtLogWarn("unparsable JSON read:%d offset:%d", doc->GetParseError(), (int) doc->GetErrorOffset());
    rtLogWarn("\"%.*s\"\n", length, buff + begin);

    return RT_FAIL;
  }

  return RT_OK;
}

rtError
rtGetPeerName(int fd, sockaddr_storage& endpoint)
{
  sockaddr_storage addr;
  memset(&addr, 0, sizeof(sockaddr_storage));

  socklen_t len;
  rtSocketGetLength(endpoint, &len);

  int ret = getpeername(fd, (sockaddr *)&addr, &len);
  if (ret == -1)
  {
    rtError err = rtErrorFromErrno(errno);
    rtLogWarn("failed to get the peername for fd:%d endpoint. %s", fd, rtStrError(err));
    return err;
  }

  memcpy(&endpoint, &addr, sizeof(sockaddr_storage));
  return RT_OK;
}

rtError
rtGetSockName(int fd, sockaddr_storage& endpoint)
{
  sockaddr_storage addr;
  memset(&addr, 0, sizeof(sockaddr_storage));

  socklen_t len;
  rtSocketGetLength(endpoint, &len);

  int ret = getsockname(fd, (sockaddr *)&addr, &len);
  if (ret == -1)
  {
    rtError err = rtErrorFromErrno(errno);
    rtLogWarn("failed to get the socket name for fd:%d endpoint. %s", fd, rtStrError(err));
    return err;
  }

  memcpy(&endpoint, &addr, sizeof(sockaddr_storage));
  return RT_OK;
}

rtError
rtCloseSocket(int& fd)
{
  if (fd != kInvalidSocket)
  {
    ::close(fd);
    fd = kInvalidSocket;
  }
  return RT_OK;
}

rtError
rtGetDefaultInterface(sockaddr_storage& addr, uint16_t port)
{
  char name[64];
  memset(name, 0, sizeof(name));

  #ifdef RT_REMOTE_LOOPBACK_ONLY
  rtError e = RT_OK;
  strcpy(name, "127.0.0.1");
  #else
  rtError e = rtFindFirstInetInterface(name, sizeof(name));
  #endif
  if (e == RT_OK)
  {
    sockaddr_storage temp;
    e = rtParseAddress(temp, name, port, nullptr);
    if (e == RT_OK)
      memcpy(&addr, &temp, sizeof(sockaddr_storage));
  }

  return e;
}

rtError
rtCreateUnixSocketName(pid_t pid, char* buff, int n)
{
  if (!buff)
    return RT_ERROR_INVALID_ARG;

  if (pid == 0)
    pid = getpid();

  int count = snprintf(buff, n, "%s.%d", kUnixSocketTemplateRoot, pid);
  if (count >= n)
  {
    rtLogError("truncated socket path %d <= %d", n, count);
    return RT_FAIL;
  }

  return RT_OK;
}

rtError
rtParseAddress(sockaddr_storage& ss, char const* s)
{
  if (!s)
    return RT_ERROR_INVALID_ARG;

  // inet:127.0.0.1:37200
  char const* p1 = strchr(s, ':');
  if (!p1)
  {
    rtLogWarn("error parsing socket address '%s'", s);
    return RT_ERROR_INVALID_ARG;
  }

  char const* p2 = strchr(++p1, ':');
  if (!p2)
  {
    rtLogWarn("error parsing socket address '%s'", s);
    return RT_ERROR_INVALID_ARG;
  }

  std::string addr(p1, (p2 - p1));
  uint16_t    port = static_cast<uint16_t>(strtol(p2 + 1, nullptr, 10));

  return rtParseAddress(ss, addr.c_str(), port, nullptr);
}
