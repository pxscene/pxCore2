#include "rtSocketUtils.h"

#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include <rtLog.h>
#include <sstream>

#include <rapidjson/memorystream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

static const int kMaxMessageLength = (1024 * 16);

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
    if (n == 0)
    {
      rtLogWarn("socket closed");
      return RT_FAIL;
    }

    if (n == -1)
    {
      rtLogError("failed to read from fd %d. %s", fd, strerror(errno));
      return RT_FAIL;;
    }
    bytes_read += n;
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

  char addr_buff[128];
  memset(addr_buff, 0, sizeof(addr_buff));
  inet_ntop(ss.ss_family, addr, addr_buff, sizeof(addr_buff));

  std::stringstream buff;
  buff << addr_buff;
  buff << ':';
  buff << port;
  return buff.str();
}

rtError
rtSendDocument(rapidjson::Document& doc, int fd, sockaddr_storage const* dest)
{
  rapidjson::StringBuffer buff;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buff);
  doc.Accept(writer);

  #ifdef RT_RPC_DEBUG
  char const* verb = (dest != NULL ? "sendto" : "send");
  rtLogDebug("%s (%d):\n\t\"%.*s\"\n", verb, static_cast<int>(buff.GetSize()),
    static_cast<int>(buff.GetSize()), buff.GetString());
  #endif

  if (dest)
  {
    socklen_t len;
    rtSocketGetLength(*dest, &len);

    if (sendto(fd, buff.GetString(), buff.GetSize(), MSG_NOSIGNAL,
          reinterpret_cast<sockaddr const *>(dest), len) < 0)
    {
      rtLogError("sendto failed. %s. dest:%s family:%d", strerror(errno), rtSocketToString(*dest).c_str(),
        dest->ss_family);

      return RT_FAIL;
    }
  }
  else
  {
    // send length first
    int n = buff.GetSize();
    n = htonl(n);

    if (send(fd, reinterpret_cast<char *>(&n), 4, MSG_NOSIGNAL) < 0)
    {
      rtLogError("failed to send length of message. %s", strerror(errno));
      return RT_FAIL;
    }

    if (send(fd, buff.GetString(), buff.GetSize(), MSG_NOSIGNAL) < 0)
    {
      rtLogError("failed to send. %s", strerror(errno));
      return RT_FAIL;
    }
  }

  return RT_OK;
}

rtError
rtReadMessage(int fd, rt_sockbuf_t& buff, rtJsonDocPtr_t& doc)
{
  rtError err = RT_OK;

  int n = 0;
  int capacity = static_cast<int>(buff.capacity());

  err = rtReadUntil(fd, reinterpret_cast<char *>(&n), 4);
  if (err != RT_OK)
  {
    rtLogWarn("error reading length from socket");
    return err;
  }

  n = ntohl(n);

  if (n > capacity)
  {
    rtLogWarn("buffer capacity %d not big enough for message size: %d", capacity, n);
    // TODO: should drain, and discard message
    assert(false);
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
  rtLogDebug("read (%d):\n\t\"%.*s\"\n", static_cast<int>(buff.size()), static_cast<int>(buff.size()), &buff[0]);
  #endif
 
  doc.reset(new rapidjson::Document());

  rapidjson::MemoryStream stream(&buff[0], n);
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
    rtLogWarn("\"%.*s\"\n", length, &buff[0] + begin);

    return RT_FAIL;
  }
  
  return RT_OK;
}

 
