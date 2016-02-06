#include "rtSocketUtils.h"

#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include <rtLog.h>

rtError
rtParseAddress(sockaddr_storage& ss, char const* addr, uint16_t port)
{
  int ret = 0;

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
        return RT_FAIL;

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
  }
  else
  {
    return RT_FAIL;
  }
  return RT_OK;
}

rtError
rtSocketGetLength(sockaddr_storage const& ss, socklen_t* len)
{
  if (!len)
    return RT_FAIL;
  *len = (ss.ss_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6));
  return RT_OK;
}

rtError
rtGetInterfaceAddress(char const* name, sockaddr_storage& ss)
{
  rtError error = RT_OK;
  ifaddrs* ifaddr = NULL;

  int ret = getifaddrs(&ifaddr);

  if (ret == -1)
  {
    int err = errno;
    rtLogError("failed to get list of interfaces. %s", strerror(err));
    return RT_FAIL;
  }

  for (ifaddrs* i = ifaddr; i != NULL; i = i->ifa_next)
  {
    if (i->ifa_addr == NULL)
      continue;

    if (strcmp(name, i->ifa_name) == 0)
    {
      char host[NI_MAXHOST];

      ss.ss_family = i->ifa_addr->sa_family;
      if (ss.ss_family != AF_INET && ss.ss_family != AF_INET6)
        continue;

      socklen_t len;
      rtSocketGetLength(ss, &len);

      ret = getnameinfo(i->ifa_addr, len, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (ret != 0)
      {
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
          rtLogError("failed to parse: %s as valid ipv4 address", host);
          error = RT_FAIL;
        }
        goto out;
      }
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

  void const* p = (ss.ss_family == AF_INET)
    ? reinterpret_cast<void const *>(&(v4->sin_addr))
    : reinterpret_cast<void const *>(&(v6->sin6_addr));

  *addr = const_cast<void *>(p);

  return RT_OK;
}

rtError
rtGetPort(sockaddr_storage const& ss, uint16_t* port)
{
  sockaddr_in const* v4 = reinterpret_cast<sockaddr_in const *>(&ss);
  sockaddr_in6 const* v6 = reinterpret_cast<sockaddr_in6 const *>(&ss);
  *port = ntohs((ss.ss_family == AF_INET) ? v4->sin_port : v6->sin6_port);
  return RT_OK;
}

rtError
rtPushFd(fd_set* fds, int fd, int* max_fd)
{
  if (fd != -1)
  {
    FD_SET(fd, fds);
    if (max_fd && fd > *max_fd)
      *max_fd = fd;
  }
  return RT_OK;
}

rtError
rtReadUntil(int fd, char* buff, int n)
{
  ssize_t bytes_read = 0;
  ssize_t bytes_to_read = n;

  while (bytes_read < bytes_to_read)
  {
    ssize_t n = read(fd, buff + bytes_read, (bytes_to_read - bytes_read));
    if (n == -1)
    {
      rtLogError("failed to read from fd %d. %s", fd, strerror(errno));
      return RT_FAIL;;
    }
    bytes_read += n;
  }
  return RT_OK;
}
