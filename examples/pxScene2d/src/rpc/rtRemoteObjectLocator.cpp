#include "rtRemoteObjectLocator.h"

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
  : m_fd(-1)
  , m_family(0)
  , m_pid(getpid())
{
  memset(&m_dest, 0, sizeof(m_dest));
  memset(&m_src, 0, sizeof(m_src));
  m_buff.reserve(1024 * 4);
  m_buff.resize(1024 * 4);
  pthread_mutex_init(&m_mutex, NULL);
}

rtRemoteObjectLocator::~rtRemoteObjectLocator()
{
  if (m_fd != -1)
  {
    close(m_fd);
  }
}

rtError
rtRemoteObjectLocator::open(char const* dstaddr, int16_t port, char const* srcaddr)
{
  set_dest(dstaddr, port);

  m_fd = socket(m_family, SOCK_DGRAM, 0);
  if (m_fd < 0)
  {
    rtLogError("failed to create socket: %s", strerror(errno));
    return RT_FAIL;
  }

  int reuse = 1;
  if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
  {
    rtLogError("failed to set reuse addr: %s", strerror(errno));
    return RT_FAIL;
  }

  #if 0
  char loop = 0;
  if (setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) < 0)
  {
    rtLogError("failed to disable multicast loopback: %s", strerror(errno));
    close(m_fd);
    m_fd = -1;
    return RT_FAIL;
  }
  #endif

  // TODO: v6
  set_src(srcaddr, port);

  sockaddr_in src;
  memset(&src, 0, sizeof(src));
  src.sin_family = AF_INET;
  src.sin_port = htons(port);
  src.sin_addr.s_addr = INADDR_ANY;
  rtLogInfo("bind: %s:%d", inet_ntoa(src.sin_addr), ntohs(src.sin_port));
  if (bind(m_fd, reinterpret_cast<sockaddr *>(&src), sizeof(sockaddr_in)) < 0)
  {
    rtLogError("failed to bind to source address: %s", strerror(errno));
    return RT_FAIL;
  }

  // join group
  rtLogInfo("joining %s on %s", dstaddr, srcaddr);

  ip_mreq group;
  group.imr_multiaddr.s_addr = inet_addr(dstaddr);
  group.imr_interface.s_addr = inet_addr(srcaddr);
  if (setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
  {
    rtLogError("failed to add to group: %s", strerror(errno));
    return RT_FAIL;
  }

  return 0;
}

int
rtRemoteObjectLocator::set_src(char const* srcaddr, int16_t port)
{
  return (m_family == AF_INET)
    ? set_src_v4(srcaddr, port)
    : set_src_v6(srcaddr, port);
}

int
rtRemoteObjectLocator::set_dest(char const* dstaddr, int16_t port)
{
  int err = 0;
  int ret = 0;

  sockaddr_in* addr = reinterpret_cast<sockaddr_in *>(&m_dest);
  ret = inet_pton(AF_INET, dstaddr, &addr->sin_addr);
  if (ret == 1)
  {
#ifndef __linux__
    addr->sin_len = sizeof(sockaddr_in);
#endif
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    m_family = AF_INET;
  }
  else if (ret == 0)
  {
    // TODO: try ipv6
    assert(false);
  }
  else
  {
    err = errno;
  }
  return err;
}

int
rtRemoteObjectLocator::set_src_v4(char const* srcaddr, int16_t port)
{
  int ret = 0;

  sockaddr_in* addr = reinterpret_cast<sockaddr_in *>(&m_src);
  ret = inet_pton(AF_INET, srcaddr, &addr->sin_addr);
  if (ret == 0)
    return errno;

  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr(srcaddr);
  addr->sin_port = htons(port);
  if (setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_IF, &addr->sin_addr, sizeof(in_addr)) < 0)
    return errno;

  return 0;
}

int
rtRemoteObjectLocator::set_src_v6(char const* /*srcaddr*/, int16_t /*port*/)
{
  assert(false);
  return 0;
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
    int maxFd = m_fd + 1;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    FD_SET(m_fd, &read_fds);

    FD_ZERO(&err_fds);
    FD_SET(m_fd, &err_fds);

    rtLogInfo("select");
    int ret = select(maxFd, &read_fds, NULL, &err_fds, NULL);
    if (ret == -1)
    {
      int err = errno;
      rtLogWarn("select failed: %s", strerror(err));
      continue;
    }

    if (FD_ISSET(m_fd, &read_fds))
      do_read();
  }
}

void
rtRemoteObjectLocator::do_read()
{
  sockaddr_in src;
  socklen_t len = sizeof(src);

  #if 0
  ssize_t n = read(m_fd, &m_buff[0], m_buff.capacity());
  #endif

  ssize_t n = recvfrom(m_fd, &m_buff[0], m_buff.capacity(), 0, reinterpret_cast<sockaddr *>(&src), &len);
  if (n > 0)
    m_buff.resize(n);

  rtLogInfo("new message from %s:%d", inet_ntoa(src.sin_addr), htons(src.sin_port));
  printf("read: \"%.*s\"\n", int(n), &m_buff[0]); // static_cast<int>(m_buff.size()), &m_buff[0]);
}

rtError
rtRemoteObjectLocator::startListener()
{
  m_run = true;
  pthread_create(&m_thread, NULL, &rtRemoteObjectLocator::run_listener, this);
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
    sockaddr_in* addr = reinterpret_cast<sockaddr_in *>(&m_dest);
    rtLogInfo("sendto: %s:%d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
    if (sendto(m_fd, req.c_str(), req.size(), MSG_NOSIGNAL, reinterpret_cast<sockaddr *>(&m_dest),
      sizeof(sockaddr_in)) < 0)
    {
      rtLogError("failed to send: %s", strerror(errno));
      return rtObjectRef();
    }
  }

  return obj;
}
