#include "rtRemoteObjectResolver.h"
#include "rtSocketUtils.h"
#include "rtRpcMessage.h"

#include <rtLog.h>

#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

rtRemoteObjectResolver::rtRemoteObjectResolver(sockaddr_storage const& rpc_endpoint)
  : m_mcast_fd(-1)
  , m_mcast_src_index(-1)
  , m_ucast_fd(-1)
  , m_ucast_len(0)
  , m_pid(getpid())
  , m_command_handlers()
  , m_rpc_addr()
  , m_rpc_port(0)
  , m_seq_id(0)
{
  memset(&m_mcast_dest, 0, sizeof(m_mcast_dest));
  memset(&m_mcast_src, 0, sizeof(m_mcast_src));
  memset(&m_ucast_endpoint, 0, sizeof(m_ucast_endpoint));

  m_command_handlers.insert(cmd_handler_map_t::value_type("search", &rtRemoteObjectResolver::onSearch));
  m_command_handlers.insert(cmd_handler_map_t::value_type("locate", &rtRemoteObjectResolver::onLocate));
    
  char addr_buff[128];

  void* addr = NULL;
  rtGetInetAddr(rpc_endpoint, &addr);

  socklen_t len;
  rtSocketGetLength(rpc_endpoint, &len);
  char const* p = inet_ntop(rpc_endpoint.ss_family, addr, addr_buff, len);
  if (p)
    m_rpc_addr = p;

  rtGetPort(rpc_endpoint, &m_rpc_port);
}

rtRemoteObjectResolver::~rtRemoteObjectResolver()
{
  if (m_mcast_fd != -1)
    close(m_mcast_fd);
  if (m_ucast_fd != -1)
    close(m_ucast_fd);
}

rtError
rtRemoteObjectResolver::open(char const* dstaddr, uint16_t port, char const* srcaddr)
{
  rtError err = RT_OK;

  rtLogInfo("opening with dest:%s (%d) src:%s", dstaddr, port, srcaddr);

  err = rtParseAddress(m_mcast_dest, dstaddr, port, nullptr);
  if (err != RT_OK)
    return err;

  // TODO: no need to parse srcaddr multiple times, just copy result
  err = rtParseAddress(m_mcast_src, srcaddr, port, &m_mcast_src_index);
  if (err != RT_OK)
    return err;

  err = rtParseAddress(m_ucast_endpoint, srcaddr, 0, nullptr);
  if (err != RT_OK)
    return err;

  err = openUnicastSocket();
  if (err != RT_OK)
    return err;

  return RT_OK;
}

rtError
rtRemoteObjectResolver::openUnicastSocket()
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

  rtSocketGetLength(m_ucast_endpoint, &m_ucast_len);

  // listen on ANY port
  ret = bind(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), m_ucast_len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to bind unicast endpoint: %s", strerror(err));
    return RT_FAIL;
  }

  // now figure out which port we're bound to
  rtSocketGetLength(m_ucast_endpoint, &m_ucast_len);
  ret = getsockname(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), &m_ucast_len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to get socketname. %s", strerror(err));
    return RT_FAIL;
  }
  else
  {
    rtLogInfo("local udp socket bound to %s", rtSocketToString(m_ucast_endpoint).c_str());
  }

  // btw, when we use this socket to send multicast, use the right interface
  if (m_mcast_src.ss_family == AF_INET)
  {
    in_addr mcast_interface = reinterpret_cast<sockaddr_in *>(&m_mcast_src)->sin_addr;
    err = setsockopt(m_ucast_fd, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<char *>(&mcast_interface),
        sizeof(mcast_interface));
  }
  else
  {
    uint32_t ifindex = m_mcast_src_index;
    err = setsockopt(m_ucast_fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex));
  }

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
rtRemoteObjectResolver::onSearch(rtJsonDocPtr_t const& doc, sockaddr_storage const& soc)
{
  auto senderId = doc->FindMember(kFieldNameSenderId);
  assert(senderId != doc->MemberEnd());
  if (senderId->value.GetInt() == m_pid)
  {
    return RT_OK;
  }

  int key = rtMessage_GetCorrelationKey(*doc);


  auto itr = m_registered_objects.end();

  char const* objectId = rtMessage_GetObjectId(*doc);

  std::unique_lock<std::mutex> lock(m_mutex);
  itr = m_registered_objects.find(objectId);
  lock.unlock();

  if (itr != m_registered_objects.end())
  {
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember(kFieldNameMessageType, "locate", doc.GetAllocator());
    doc.AddMember(kFieldNameObjectId, std::string(objectId), doc.GetAllocator());
    doc.AddMember("ip", m_rpc_addr, doc.GetAllocator());
    doc.AddMember("port", m_rpc_port, doc.GetAllocator());
    // echo kback to sender
    doc.AddMember(kFieldNameSenderId, senderId->value.GetInt(), doc.GetAllocator());
    doc.AddMember(kFieldNameCorrelationKey, key, doc.GetAllocator());

    return rtSendDocument(doc, m_ucast_fd, &soc);
  }

  return RT_OK;
}

rtError
rtRemoteObjectResolver::onLocate(rtJsonDocPtr_t const& doc, sockaddr_storage const& /*soc*/)
{
  int key = rtMessage_GetCorrelationKey(*doc);

  std::unique_lock<std::mutex> lock(m_mutex);
  m_pending_searches[key] = doc;
  lock.unlock();
  m_cond.notify_all();

  return RT_OK;
}

rtError
rtRemoteObjectResolver::resolveObject(std::string const& name, sockaddr_storage& endpoint, uint32_t timeout)
{
  rtError err = RT_OK;
  rtAtomic seqId = rtAtomicInc(&m_seq_id);

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember(kFieldNameMessageType, "search", doc.GetAllocator());
  doc.AddMember(kFieldNameObjectId, name, doc.GetAllocator());
  doc.AddMember(kFieldNameSenderId, m_pid, doc.GetAllocator());
  doc.AddMember(kFieldNameCorrelationKey, seqId, doc.GetAllocator());

  err = rtSendDocument(doc, m_ucast_fd, &m_mcast_dest);
  if (err != RT_OK)
    return err;

  rtJsonDocPtr_t searchResponse;
  request_map_t::const_iterator itr;

  auto delay = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);

  // wait here until timeout expires or we get a response that matches out pid/seqid
  std::unique_lock<std::mutex> lock(m_mutex);
  m_cond.wait_until(lock, delay, [this, seqId, &searchResponse]
    {
      auto itr = this->m_pending_searches.find(seqId);
      if (itr != this->m_pending_searches.end())
      {
        searchResponse = itr->second;
        this->m_pending_searches.erase(itr);
      }
      return searchResponse != nullptr;
    });
  lock.unlock();

  if (!searchResponse)
    return RT_FAIL;

  // response is in itr
  if (searchResponse)
  {
    assert(searchResponse->HasMember("ip"));
    assert(searchResponse->HasMember("port"));

    rtError err = rtParseAddress(endpoint, (*searchResponse)["ip"].GetString(),
        (*searchResponse)["port"].GetInt(), nullptr);

    if (err != RT_OK)
      return err;
  }

  return RT_OK;
}

void
rtRemoteObjectResolver::runListener()
{
  buff_t buff;
  buff.reserve(1024 * 1024);
  buff.resize(1024 * 1024);

  while (true)
  {
    int maxFd = 0;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    rtPushFd(&read_fds, m_mcast_fd, &maxFd);
    rtPushFd(&read_fds, m_ucast_fd, &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_mcast_fd, &maxFd);
    rtPushFd(&err_fds, m_ucast_fd, &maxFd);

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, NULL);
    if (ret == -1)
    {
      int err = errno;
      rtLogWarn("select failed: %s", strerror(err));
      continue;
    }

    if (FD_ISSET(m_mcast_fd, &read_fds))
      doRead(m_mcast_fd, buff);

    if (FD_ISSET(m_ucast_fd, &read_fds))
      doRead(m_ucast_fd, buff);
  }
}

void
rtRemoteObjectResolver::doRead(int fd, buff_t& buff)
{
  // we only suppor v4 right now. not sure how recvfrom supports v6 and v4
  sockaddr_storage src;
  socklen_t len = sizeof(sockaddr_in);

  #if 0
  ssize_t n = read(m_mcast_fd, &m_read_buff[0], m_read_buff.capacity());
  #endif

  ssize_t n = recvfrom(fd, &buff[0], buff.capacity(), 0, reinterpret_cast<sockaddr *>(&src), &len);
  if (n > 0)
    doDispatch(&buff[0], static_cast<int>(n), &src);
}

void
rtRemoteObjectResolver::doDispatch(char const* buff, int n, sockaddr_storage* peer)
{
  // rtLogInfo("new message from %s:%d", inet_ntoa(src.sin_addr), htons(src.sin_port));
  // printf("read: %d\n", int(n));
  #ifdef RT_RPC_DEBUG
  rtLogDebug("read:\n\t\"%.*s\"\n", n, buff); // static_cast<int>(m_read_buff.size()), &m_read_buff[0]);
  #endif

  rtJsonDocPtr_t doc;
  rtError err = rtParseMessage(buff, n, doc);
  if (err != RT_OK)
    return;

  char const* message_type = rtMessage_GetMessageType(*doc);

  auto itr = m_command_handlers.find(message_type);
  if (itr == m_command_handlers.end())
  {
    rtLogWarn("no command handler registered for: %s", message_type);
    return;
  }

  // https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn
  #define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

  err = CALL_MEMBER_FN(*this, itr->second)(doc, *peer);
  if (err != RT_OK)
  {
    rtLogWarn("failed to run command for %s. %d", message_type, err);
    return;
  }
}

rtError
rtRemoteObjectResolver::start()
{
  rtError err = openMulticastSocket();
  if (err != RT_OK)
    return err;
  m_read_thread.reset(new std::thread(&rtRemoteObjectResolver::runListener, this));
  return RT_OK;
}

rtError
rtRemoteObjectResolver::registerObject(std::string const& name)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_registered_objects.insert(name);
  return RT_OK;
}

rtError
rtRemoteObjectResolver::openMulticastSocket()
{
  int err = 0;

  m_mcast_fd = socket(m_mcast_dest.ss_family, SOCK_DGRAM, 0);
  if (m_mcast_fd < 0)
  {
    err = errno;
    rtLogError("failed to create datagram socket with family:%d. %s",
      m_mcast_dest.ss_family, strerror(err));
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

  if (m_mcast_src.ss_family == AF_INET)
  {
    sockaddr_in saddr = *(reinterpret_cast<sockaddr_in *>(&m_mcast_src));
    saddr.sin_addr.s_addr = INADDR_ANY;
    err = bind(m_mcast_fd, reinterpret_cast<sockaddr *>(&saddr), sizeof(sockaddr_in));
  }
  else
  {
    sockaddr_in6* v6 = reinterpret_cast<sockaddr_in6 *>(&m_mcast_src);
    v6->sin6_addr = in6addr_any;
    err = bind(m_mcast_fd, reinterpret_cast<sockaddr *>(v6), sizeof(sockaddr_in6));
  }

  if (err < 0)
  {
    err = errno;
    rtLogError("failed to bind multicast socket to %s. %s",
        rtSocketToString(m_mcast_src).c_str(),  strerror(err));
    return RT_FAIL;
  }

  // join group
  if (m_mcast_src.ss_family == AF_INET)
  {
    ip_mreq group;
    group.imr_multiaddr = reinterpret_cast<sockaddr_in *>(&m_mcast_dest)->sin_addr;
    group.imr_interface = reinterpret_cast<sockaddr_in *>(&m_mcast_src)->sin_addr;

    err = setsockopt(m_mcast_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group));
  }
  else
  {
    ipv6_mreq mreq;
    mreq.ipv6mr_multiaddr = reinterpret_cast<sockaddr_in6 *>(&m_mcast_dest)->sin6_addr;
    mreq.ipv6mr_interface = m_mcast_src_index;
    err = setsockopt(m_mcast_fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq));
  }

  if (err < 0)
  {
    err = errno;
    rtLogError("failed to join mcast group %s. %s", rtSocketToString(m_mcast_dest).c_str(),
      strerror(err));
    return RT_FAIL;
  }

  rtLogInfo("successfully joined multicast group: %s on interface: %s",
    rtSocketToString(m_mcast_dest).c_str(), rtSocketToString(m_mcast_src).c_str());

  return RT_OK;
}
