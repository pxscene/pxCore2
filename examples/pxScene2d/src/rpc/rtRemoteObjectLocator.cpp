#include "rtRemoteObjectLocator.h"

#include <netdb.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <rtLog.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

static rtError
get_interface_address(char const* name, sockaddr_storage* sa)
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

      sa->ss_family = i->ifa_addr->sa_family;
      if (sa->ss_family != AF_INET && sa->ss_family != AF_INET6)
        continue;

      size_t len = (sa->ss_family == AF_INET) ? sizeof(sockaddr_in)
        : sizeof(sockaddr_in6);

      ret = getnameinfo(i->ifa_addr, len, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (ret != 0)
      {
        rtLogError("failed to get address for %s. %s", name, gai_strerror(ret));
        error = RT_FAIL;
        goto out;
      }
      else
      {
        if (sa->ss_family == AF_INET)
        {
          sockaddr_in* v4 = reinterpret_cast<sockaddr_in *>(sa);
          ret = inet_pton(AF_INET, host, &v4->sin_addr);
          if (ret != 1)
          {
            rtLogError("failed to parse: %s as valid ipv4 address", host);
            error = RT_FAIL;
          }
        }
        if (sa->ss_family == AF_INET6)
        {
          // TODO
          assert(false);
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

static void
push_fd(fd_set* fds, int fd, int* max_fd)
{
  if (fd != -1)
  {
    FD_SET(fd, fds);
    if (max_fd && fd > *max_fd)
      *max_fd = fd;
  }
}

static socklen_t
get_length(sockaddr_storage const& ss)
{
  return ss.ss_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
}

static int
parse_address(sockaddr_storage& ss, char const* addr, uint16_t port)
{
  int ret = 0;
  int err = 0;

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
  }
  else if (ret == 0)
  {
    sockaddr_in6* v6 = reinterpret_cast<sockaddr_in6 *>(&ss);
    ret = inet_pton(AF_INET6, addr, &v6->sin6_addr);
    if (ret == 0)
    {
      // try hostname
      rtError err = get_interface_address(addr, &ss);
      if (err != RT_OK)
        err = EINVAL;
      if (ss.ss_family == AF_INET)
      {
        v4->sin_family = AF_INET;
        v4->sin_port = htons(port);
      }
    }
  }
  else
  {
    err = errno;
  }
  return err;
}

static std::string
createSearchRequest(std::string const& name, pid_t pid)
{
  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember("object-id", name, doc.GetAllocator());
  doc.AddMember("type", "search", doc.GetAllocator());
  doc.AddMember("source-id", pid, doc.GetAllocator());

  rapidjson::GenericStringBuffer<rapidjson::UTF8<> > buff;
  rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<> > > writer(buff);
  doc.Accept(writer);
  return std::string(buff.GetString(), buff.GetSize());
}

rtRemoteObjectLocator::rtRemoteObjectLocator()
  : m_mcast_fd(-1)
  , m_mcast_family(0)
  , m_ucast_fd(-1)
  , m_ucast_family(0)
  , m_read_thread(0)
  , m_read_run(false)
  , m_pid(getpid())
{
  memset(&m_mcast_dest, 0, sizeof(m_mcast_dest));
  memset(&m_mcast_src, 0, sizeof(m_mcast_src));
  memset(&m_ucast_endpoint, 0, sizeof(m_ucast_endpoint));
  m_read_buff.reserve(1024 * 4);
  m_read_buff.resize(1024 * 4);
  pthread_mutex_init(&m_mutex, NULL);
}

rtRemoteObjectLocator::~rtRemoteObjectLocator()
{
  if (m_mcast_fd != -1)
    close(m_mcast_fd);
  if (m_ucast_fd != -1)
    close(m_ucast_fd);
}

rtError
rtRemoteObjectLocator::open(char const* dstaddr, int16_t port, char const* srcaddr)
{
  rtError err = RT_OK;

  err = parse_address(m_mcast_dest, dstaddr, port);
  if (err != RT_OK)
    return err;

  err = parse_address(m_mcast_src, srcaddr, port);
  if (err != RT_OK)
    return err;

  err = parse_address(m_ucast_endpoint, srcaddr, 0);
  if (err != RT_OK)
    return err;

  err = open_unicast_socket();
  if (err != RT_OK)
    return err;

  return RT_OK;
}

rtError
rtRemoteObjectLocator::open_multicast_socket()
{
  int err = 0;

  m_mcast_fd = socket(m_mcast_dest.ss_family, SOCK_DGRAM, 0);
  if (m_mcast_fd < 0)
  {
    err = errno;
    rtLogError("failed to create multicast socket. %s", strerror(err));
    return RT_FAIL;
  }

  // re-use because multiple applications may want to join group on same machine
  int reuse = 1;
  err = setsockopt(m_mcast_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to set reuseaddr. %s", strerror(err));
    return RT_FAIL;
  }

  sockaddr_in saddr = *(reinterpret_cast<sockaddr_in *>(&m_mcast_src));
  saddr.sin_addr.s_addr = INADDR_ANY;
  err = bind(m_mcast_fd, reinterpret_cast<sockaddr *>(&saddr), sizeof(sockaddr_in));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to bind socket. %s", strerror(err));
    return RT_FAIL;
  }

  // join group
  ip_mreq group;
  group.imr_multiaddr = reinterpret_cast<sockaddr_in *>(&m_mcast_dest)->sin_addr;
  group.imr_interface = reinterpret_cast<sockaddr_in *>(&m_mcast_src)->sin_addr;

  err = setsockopt(m_mcast_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to join mcast group. %s", strerror(err));
    return RT_FAIL;
  }

  rtLogInfo("successfully joined multicast group: %s on interface: %s",
      inet_ntoa(group.imr_multiaddr), inet_ntoa(group.imr_interface));

  return RT_OK;
}

rtError
rtRemoteObjectLocator::open_unicast_socket()
{
  int ret = 0;
  int err = 0;

  m_ucast_fd = socket(m_ucast_endpoint.ss_family, SOCK_DGRAM, 0);
  if (m_ucast_fd < 0)
  {
    err = errno;
    rtLogError("failed to create unicast socket with family: %d. %s", 
      m_ucast_endpoint.ss_family, strerror(err));
    return RT_FAIL;
  }

  // listen on ANY port
  ret = bind(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), get_length(m_ucast_endpoint));
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to bind unicast endpoint: %s", strerror(err));
    return RT_FAIL;
  }

  // now figure out which port we're bound to
  socklen_t len = get_length(m_ucast_endpoint);
  ret = getsockname(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), &len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to get socketname. %s", strerror(err));
    return RT_FAIL;
  }
  else
  {
    sockaddr_in* saddr = reinterpret_cast<sockaddr_in *>(&m_ucast_endpoint);
    rtLogInfo("local udp socket bound to %s:%d", inet_ntoa(saddr->sin_addr), ntohs(saddr->sin_port));
  }

  // btw, when we use this socket to send multicast, use the right interface
  in_addr mcast_interface = reinterpret_cast<sockaddr_in *>(&m_mcast_src)->sin_addr;
  err = setsockopt(m_ucast_fd, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<char *>(&mcast_interface),
    sizeof(mcast_interface));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to set outgoing multicast interface. %s", strerror(err));
    return RT_FAIL;
  }

  #if 0
  char loop = 1;
  if (setsockopt(m_ucast_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) < 0)
  {
    rtLogError("failed to disable multicast loopback: %s", strerror(errno));
    return RT_FAIL;
  }
  #endif

  return RT_OK;
}

rtError
rtRemoteObjectLocator::registerObject(std::string const& name, rtObjectRef const& obj)
{
  pthread_mutex_lock(&m_mutex);
  auto itr = m_objects.find(name);
  if (itr != m_objects.end())
  {
    pthread_mutex_unlock(&m_mutex);
    rtLogWarn("object %s is already registered", name.c_str());
    return EEXIST;
  }
  m_objects.insert(refmap_t::value_type(name, obj));
  pthread_mutex_unlock(&m_mutex);
  return RT_OK;
}

void*
rtRemoteObjectLocator::run_listener(void* argp)
{
  rtRemoteObjectLocator* locator = reinterpret_cast<rtRemoteObjectLocator *>(argp);
  locator->run_listener();
  return NULL;
}

void
rtRemoteObjectLocator::run_listener()
{
  rtLogInfo("running listener");

  while (true)
  {
    int maxFd = m_mcast_fd + 1;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    push_fd(&read_fds, m_mcast_fd, &maxFd);
    push_fd(&read_fds, m_ucast_fd, &maxFd);

    FD_ZERO(&err_fds);
    push_fd(&err_fds, m_mcast_fd, &maxFd);
    push_fd(&err_fds, m_ucast_fd, &maxFd);

    int ret = select(maxFd, &read_fds, NULL, &err_fds, NULL);
    if (ret == -1)
    {
      int err = errno;
      rtLogWarn("select failed: %s", strerror(err));
      continue;
    }

    if (FD_ISSET(m_mcast_fd, &read_fds))
      do_read(m_mcast_fd);

    if (FD_ISSET(m_ucast_fd, &read_fds))
      do_read(m_ucast_fd);
  }
}

void
rtRemoteObjectLocator::do_read(int fd)
{
  sockaddr_in src;
  socklen_t len = sizeof(src);

  #if 0
  ssize_t n = read(m_mcast_fd, &m_read_buff[0], m_read_buff.capacity());
  #endif

  ssize_t n = recvfrom(fd, &m_read_buff[0], m_read_buff.capacity(), 0, reinterpret_cast<sockaddr *>(&src), &len);
  if (n > 0)
    m_read_buff.resize(n);

  rtLogInfo("new message from %s:%d", inet_ntoa(src.sin_addr), htons(src.sin_port));
  printf("read: \"%.*s\"\n", int(n), &m_read_buff[0]); // static_cast<int>(m_read_buff.size()), &m_read_buff[0]);
}

rtError
rtRemoteObjectLocator::startListener()
{
  rtError err = open_multicast_socket();
  if (err != RT_OK)
    return err;

  pthread_create(&m_read_thread, NULL, &rtRemoteObjectLocator::run_listener, this);
  return RT_OK;
}

rtObjectRef
rtRemoteObjectLocator::findObject(std::string const& name)
{
  rtObjectRef obj;

  // first check local
  auto itr = m_objects.find(name);
  if (itr != m_objects.end())
    obj = itr->second;

  // if object is not registered with us locally, then check network
  if (!obj)
  {
    rtLogDebug("sending out mutlicast search for: %s", name.c_str());

    std::string req = createSearchRequest(name, m_pid);
    sockaddr_in* addr = reinterpret_cast<sockaddr_in *>(&m_mcast_dest);
    rtLogInfo("sendto: %s:%d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));

    if (sendto(m_ucast_fd, req.c_str(), req.size(), MSG_NOSIGNAL,
      reinterpret_cast<sockaddr *>(&m_mcast_dest), get_length(m_mcast_dest)) < 0)
    {
      rtLogError("failed to send: %s", strerror(errno));
      return rtObjectRef();
    }
  }

  return obj;
}
