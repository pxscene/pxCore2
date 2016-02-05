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
createSearchRequest(std::string const& name)
{
  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember("object-id", name, doc.GetAllocator());
  doc.AddMember("type", "search", doc.GetAllocator());

  rapidjson::GenericStringBuffer<rapidjson::UTF8<> > buff;
  rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<> > > writer(buff);
  doc.Accept(writer);
  return std::string(buff.GetString(), buff.GetSize());
}

rtRemoteObjectLocator::rtRemoteObjectLocator()
  : m_fd(-1)
  , m_family(0)
{
  memset(&m_dest, 0, sizeof(m_dest));
  memset(&m_src, 0, sizeof(m_src));
}

rtRemoteObjectLocator::~rtRemoteObjectLocator()
{
  if (m_fd != -1)
  {
    close(m_fd);
  }
}

rtError
rtRemoteObjectLocator::open(char const* dstaddr, int16_t dstport, char const* srcaddr)
{
  set_dest(dstaddr, dstport);

  m_fd = socket(m_family, SOCK_DGRAM, 0);
  if (m_fd < 0)
  {
    rtLogError("failed to create socket: %s", strerror(errno));
    return RT_FAIL;
  }

  char loop = 0;
  if (setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) < 0)
  {
    rtLogError("failed to disable multicast loopback: %s", strerror(errno));
    close(m_fd);
    m_fd = -1;
    return RT_FAIL;
  }

  // TODO: v6
  sockaddr_in* src = reinterpret_cast<sockaddr_in *>(&m_src);
  if (bind(m_fd, reinterpret_cast<sockaddr *>(src), sizeof(sockaddr_in)) < 0)
  {
    rtLogError("failed to bind to source address: %s", strerror(errno));
    return RT_FAIL;
  }

  // join group
  ip_mreq group;
  group.imr_multiaddr.s_addr = inet_addr(dstaddr);
  group.imr_interface.s_addr = inet_addr(srcaddr);
  if (setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
  {
    rtLogError("failed to add to group: %s", strerror(errno));
    return RT_FAIL;
  }

  set_src(srcaddr);

  return 0;
}

int
rtRemoteObjectLocator::set_src(char const* srcaddr)
{
  return (m_family == AF_INET)
    ? set_src_v4(srcaddr)
    : set_src_v6(srcaddr);
}

int
rtRemoteObjectLocator::set_dest(char const* dstaddr, int16_t dstport)
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
    addr->sin_port = htons(dstport);
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
rtRemoteObjectLocator::set_src_v4(char const* srcaddr)
{
  int ret = 0;

  sockaddr_in* addr = reinterpret_cast<sockaddr_in *>(&m_src);
  ret = inet_pton(AF_INET, srcaddr, &addr->sin_addr);
  if (ret == 0)
    return errno;

  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr(srcaddr);
  if (setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_IF, &addr->sin_addr, sizeof(in_addr)) < 0)
    return errno;

  return 0;
}

int
rtRemoteObjectLocator::set_src_v6(char const* /*srcaddr*/)
{
  assert(false);
  return 0;
}

rtError
rtRemoteObjectLocator::registerObject(std::string const& name, rtObjectRef const& obj)
{
  auto itr = m_objects.find(name);
  if (itr != m_objects.end())
  {
    rtLogWarn("object %s is already registered", name.c_str());
    return EEXIST;
  }
  else
  {
    rtLogInfo("object %s is now registered", name.c_str());
  }

  m_objects.insert(refmap_t::value_type(name, obj));
  return RT_OK;
}

rtError
rtRemoteObjectLocator::startListener()
{
  // pthread_creat();
}

rtObjectRef
rtRemoteObjectLocator::findObject(std::string const& name)
{
  rtObjectRef obj;

  // first check local
  auto itr = m_objects.find(name);
  if (itr != m_objects.end())
    obj = itr->second;

  if (!obj)
  {
    rtLogDebug("sending out mutlicast search for: %s", name.c_str());
    
    std::string req = createSearchRequest(name);
    if (sendto(m_fd, req.c_str(), req.size(), MSG_NOSIGNAL, reinterpret_cast<sockaddr *>(&m_dest),
      sizeof(sockaddr_in)) < 0)
    {
      rtLogError("failed to send: %s", strerror(errno));
      return rtObjectRef();
    }
  }

  return obj;
}
