#include "rtRemoteMulticastResolver.h"
#include "rtRemoteSocketUtils.h"
#include "rtRemoteMessage.h"
#include "rtRemoteConfig.h"
#include "rtRemoteEnvironment.h"

#include <condition_variable>
#include <thread>
#include <mutex>

#include <rtLog.h>

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/pointer.h>

rtRemoteMulticastResolver::rtRemoteMulticastResolver(rtRemoteEnvironment* env)
  : m_mcast_fd(kInvalidSocket)
  , m_mcast_src_index(-1)
  , m_ucast_fd(kInvalidSocket)
  , m_ucast_len(0)
  , m_pid(getpid())
  , m_command_handlers()
  , m_shutdown(false)
  , m_env(env)
{
  memset(&m_mcast_dest, 0, sizeof(m_mcast_dest));
  memset(&m_mcast_src, 0, sizeof(m_mcast_src));
  memset(&m_ucast_endpoint, 0, sizeof(m_ucast_endpoint));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeSearch, &rtRemoteMulticastResolver::onSearch));
  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeLocate, &rtRemoteMulticastResolver::onLocate));
}

rtRemoteMulticastResolver::~rtRemoteMulticastResolver()
{
  rtError err = this->close();
  if (err != RT_OK)
    rtLogWarn("failed to close resolver: %s", rtStrError(err));
}

rtError rtRemoteMulticastResolver::sendSearchAndWait(const std::string& name, const int timeout, rtRemoteMessagePtr& response)
{

  rtRemoteCorrelationKey seqId = rtMessage_GetNextCorrelationKey();

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember(kFieldNameMessageType, kMessageTypeSearch, doc.GetAllocator());
  doc.AddMember(kFieldNameObjectId, name, doc.GetAllocator());
  doc.AddMember(kFieldNameSenderId, m_pid, doc.GetAllocator());
  doc.AddMember(kFieldNameCorrelationKey, seqId.toString(), doc.GetAllocator());

  // m_ucast_endpoint
  {
    std::unique_ptr<rtRemoteIPEndPoint> responseEndpoint(rtRemoteIPEndPoint::
      fromSockAddr("udp", m_ucast_endpoint));
    doc.AddMember(kFieldNameReplyTo, responseEndpoint->toString(), doc.GetAllocator());
  }


  rtRemoteMessagePtr searchResponse;
  RequestMap::const_iterator itr;

  using namespace std::chrono;
  auto iterationIncrement = milliseconds(m_env->Config->resolver_spin_init_ms());
  auto iterationTime = system_clock::now() + iterationIncrement;
  auto timeout_ = system_clock::now() + milliseconds(timeout);

  // wait here until timeout expires or we get a response that matches out pid/seqid
  std::unique_lock<std::mutex> lock(m_mutex);
  itr = m_pending_searches.end();
  do {

    rtError err = rtSendDocument(doc, m_ucast_fd, &m_mcast_dest);
    if (err != RT_OK)
      return err;

    rtLogInfo("Spinning for %llu ms", static_cast<long long unsigned int>(milliseconds(iterationIncrement).count()));

    m_cond.wait_until(lock, iterationTime, [this, seqId, &searchResponse] {
      auto itr = this->m_pending_searches.find(seqId);
      if (itr != this->m_pending_searches.end()) {
        searchResponse = itr->second;
        this->m_pending_searches.erase(itr);
      }
      return searchResponse != nullptr;
    });

    if (searchResponse)
    {
      rtLogInfo("Search response received for %s", seqId.toString().c_str());
      break;
    }

    iterationIncrement += milliseconds(m_env->Config->resolver_spin_iteration_ms());
    iterationTime = system_clock::now() + iterationIncrement;

  } while (system_clock::now() < timeout_);

  lock.unlock();

  response = searchResponse;
  return searchResponse ? RT_OK : RT_RESOURCE_NOT_FOUND;
}

rtError
rtRemoteMulticastResolver::init()
{
  rtError err = RT_OK;

  uint16_t const port = m_env->Config->resolver_multicast_port();
  std::string dstaddr = m_env->Config->resolver_multicast_address();
  std::string srcaddr = m_env->Config->resolver_multicast_interface();

  err = rtParseAddress(m_mcast_dest, dstaddr.c_str(), port, nullptr);
  if (err != RT_OK)
  {
    rtLogWarn("failed to parse address: %s. %s", dstaddr.c_str(), rtStrError(err));
    return err;
  }


  err = rtParseAddress(m_mcast_src, srcaddr.c_str(), port, &m_mcast_src_index);
  if (err != RT_OK)
  {
    err = rtGetDefaultInterface(m_mcast_src, port);
    if (err != RT_OK)
      return err;
  }

  err = rtParseAddress(m_ucast_endpoint, srcaddr.c_str(), 0, nullptr);
  if (err != RT_OK)
  {
    err = rtGetDefaultInterface(m_ucast_endpoint, 0);
    if (err != RT_OK)
      return err;
  }

  rtLogInfo("opening with dest:%s src:%s",
    rtSocketToString(m_mcast_dest).c_str(),
    rtSocketToString(m_mcast_src).c_str());

  return RT_OK;
}

rtError
rtRemoteMulticastResolver::open(sockaddr_storage const& rpc_endpoint)
{
  {
    if (rpc_endpoint.ss_family == AF_UNIX)
      m_rpc_endpoint.reset(rtRemoteFileEndPoint::fromSockAddr(rpc_endpoint));
    else
      m_rpc_endpoint.reset(rtRemoteIPEndPoint::fromSockAddr("tcp", rpc_endpoint));
  }

  rtError err = init();
  if (err != RT_OK)
  {
    rtLogWarn("failed to initialize resolver. %s", rtStrError(err));
    return err;
  }

  err = openUnicastSocket();
  if (err != RT_OK)
  {
    rtLogWarn("failed to open unicast socket. %s", rtStrError(err));
    return err;
  }

  err = openMulticastSocket();
  if (err != RT_OK)
  {
    rtLogWarn("failed to open multicast socket. %s", rtStrError(err));
    return err;
  }

  m_shutdown = false;

  m_read_thread.reset(new std::thread(&rtRemoteMulticastResolver::runListener, this));
  return RT_OK;
}

rtError
rtRemoteMulticastResolver::openMulticastSocket()
{
  int err = 0;

  m_mcast_fd = socket(m_mcast_dest.ss_family, SOCK_DGRAM, 0);
  if (NET_FAILED(m_mcast_fd))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to create datagram socket with family:%d. %s",
      m_mcast_dest.ss_family, rtStrError(e));
    return e;
  }

  // re-use because multiple applications may want to join group on same machine
  int reuse = 1;
  err = setsockopt(m_mcast_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
  if (NET_FAILED(err))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to set reuseaddr. %s", rtStrError(e));
    return e;
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

  if (NET_FAILED(err))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to bind multicast socket to %s. %s",
        rtSocketToString(m_mcast_src).c_str(),  rtStrError(e));
    return e;
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
    err = setsockopt(m_mcast_fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *)&mreq, sizeof(mreq));
  }

  if (NET_FAILED(err))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to join mcast group %s. %s", rtSocketToString(m_mcast_dest).c_str(),
      rtStrError(e));
    return e;
  }

  rtLogInfo("successfully joined multicast group: %s on interface: %s",
    rtSocketToString(m_mcast_dest).c_str(), rtSocketToString(m_mcast_src).c_str());

  return RT_OK;
}

rtError
rtRemoteMulticastResolver::openUnicastSocket()
{
  int ret = 0;

  m_ucast_fd = socket(m_ucast_endpoint.ss_family, SOCK_DGRAM, 0);
  if (NET_FAILED(m_ucast_fd))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to create unicast socket with family: %d. %s", 
      m_ucast_endpoint.ss_family, rtStrError(e));
    return e;
  }

  rtSocketGetLength(m_ucast_endpoint, &m_ucast_len);

  // listen on ANY port
  ret = bind(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), m_ucast_len);
  if (NET_FAILED(ret))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to bind unicast endpoint: %s", rtStrError(e));
    return e;
  }

  // now figure out which port we're bound to
  rtSocketGetLength(m_ucast_endpoint, &m_ucast_len);
  ret = getsockname(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), &m_ucast_len);
  if (NET_FAILED(ret))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to get socketname. %s", rtStrError(e));
    return e;
  }
  else
  {
    rtLogInfo("local udp socket bound to %s", rtSocketToString(m_ucast_endpoint).c_str());
  }

  // btw, when we use this socket to send multicast, use the right interface
  int err = 0;
  if (m_mcast_src.ss_family == AF_INET)
  {
    in_addr mcast_interface = reinterpret_cast<sockaddr_in *>(&m_mcast_src)->sin_addr;
    err = setsockopt(m_ucast_fd, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<char *>(&mcast_interface),
        sizeof(mcast_interface));
  }
  else
  {
    uint32_t ifindex = m_mcast_src_index;
    err = setsockopt(m_ucast_fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char *)&ifindex, sizeof(ifindex));
  }

  if (NET_FAILED(err))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to set outgoing multicast interface. %s", rtStrError(e));
    return e;
  }

  #if 0
  char loop = 1;
  if (setsockopt(m_ucast_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) < 0)
  {
    rtError err = rtErrorFromErrno(net_errno());
    rtLogError("failed to disable multicast loopback: %s", rtStrError(err));
    return err;
  }
  #endif

  return RT_OK;
}

rtError
rtRemoteMulticastResolver::onSearch(rtRemoteMessagePtr const& doc, sockaddr_storage const& /*soc*/)
{
  auto senderId = doc->FindMember(kFieldNameSenderId);
  RT_ASSERT(senderId != doc->MemberEnd());
  if (senderId->value.GetInt() == m_pid)
  {
    return RT_OK;
  }

  auto replyTo = doc->FindMember(kFieldNameReplyTo);
  RT_ASSERT(replyTo != doc->MemberEnd());

  rtRemoteCorrelationKey key = rtMessage_GetCorrelationKey(*doc);

  auto itr = m_hosted_objects.end();

  char const* objectId = rtMessage_GetObjectId(*doc);

  std::unique_lock<std::mutex> lock(m_mutex);
  itr = m_hosted_objects.find(objectId);
  lock.unlock();

  if (itr != m_hosted_objects.end())
  {
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember(kFieldNameMessageType, kMessageTypeLocate, doc.GetAllocator());
    doc.AddMember(kFieldNameObjectId, std::string(objectId), doc.GetAllocator());
    doc.AddMember(kFieldNameEndPoint, m_rpc_endpoint->toString(), doc.GetAllocator());
    doc.AddMember(kFieldNameSenderId, senderId->value.GetInt(), doc.GetAllocator());
    doc.AddMember(kFieldNameCorrelationKey, key.toString(), doc.GetAllocator());

    std::unique_ptr<rtRemoteEndPoint> tempEndPoint(rtRemoteEndPoint::fromString(
      replyTo->value.GetString()));
    sockaddr_storage replyToAddress = tempEndPoint->toSockAddr();

    return rtSendDocument(doc, m_ucast_fd, &replyToAddress);
  }
  
  return RT_OK;
}

rtError
rtRemoteMulticastResolver::onLocate(rtRemoteMessagePtr const& doc, sockaddr_storage const& /*soc*/)
{
  rtRemoteCorrelationKey key = rtMessage_GetCorrelationKey(*doc);

  std::unique_lock<std::mutex> lock(m_mutex);
  m_pending_searches[key] = doc;
  lock.unlock();
  m_cond.notify_all();

  return RT_OK;
}

rtError
rtRemoteMulticastResolver::locateObject(std::string const& name, sockaddr_storage& endpoint, uint32_t timeout)
{
  if (m_ucast_fd == -1)
  {
    rtLogError("unicast socket not opened");
    return RT_FAIL;
  }

  rtRemoteMessagePtr searchResponse;
  rtError err = sendSearchAndWait(name, timeout, searchResponse);
  if (err != RT_OK)
    return err;

  auto rpc_endpoint = searchResponse->FindMember(kFieldNameEndPoint);
  if (rpc_endpoint != searchResponse->MemberEnd())
  {
    std::unique_ptr<rtRemoteEndPoint> e(rtRemoteEndPoint::fromString(rpc_endpoint->value.GetString()));
    endpoint = e->toSockAddr();
    // err = rtParseAddress(endpoint, e.host().c_str(), e.port(), nullptr);
  }

  return err;
}

void
rtRemoteMulticastResolver::runListener()
{
  rtRemoteSocketBuffer buff;
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

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, &timeout);
    if (NET_FAILED(ret))
    {
      rtError e = rtErrorFromErrno(net_errno());
      rtLogWarn("select failed: %s", rtStrError(e));
      continue;
    }

    if (m_shutdown)
    {
      rtLogInfo("got shutdown signal");
      return;
    }

    if (FD_ISSET(m_mcast_fd, &read_fds))
      doRead(m_mcast_fd, buff);

    if (FD_ISSET(m_ucast_fd, &read_fds))
      doRead(m_ucast_fd, buff);
  }
}

void
rtRemoteMulticastResolver::doRead(socket_t fd, rtRemoteSocketBuffer& buff)
{
  // we only suppor v4 right now. not sure how recvfrom supports v6 and v4
  sockaddr_storage src;
  socklen_t len = sizeof(sockaddr_in);

  #if 0
  int n = read(m_mcast_fd, &m_read_buff[0], m_read_buff.capacity());
  #endif

  int n = recvfrom(fd, &buff[0], buff.capacity(), 0, reinterpret_cast<sockaddr *>(&src), &len);
  if (n > 0)
    doDispatch(&buff[0], static_cast<int>(n), &src);
}

void
rtRemoteMulticastResolver::doDispatch(char const* buff, int n, sockaddr_storage* peer)
{
  // rtLogInfo("new message from %s:%d", inet_ntoa(src.sin_addr), htons(src.sin_port));
  // printf("read: %d\n", int(n));
  #ifdef RT_RPC_DEBUG
  rtLogDebug("read:\n***IN***\t\"%.*s\"\n", n, buff); // static_cast<int>(m_read_buff.size()), &m_read_buff[0]);
  #endif

  rtRemoteMessagePtr doc;
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
rtRemoteMulticastResolver::close()
{
  if (!m_shutdown)
  {
     m_shutdown = true;

    if (m_read_thread)
    {
      m_read_thread->join();
      m_read_thread.reset();
    }
  }

  if (m_mcast_fd != -1)
    rtCloseSocket(m_mcast_fd);
  if (m_ucast_fd != -1)
    rtCloseSocket(m_ucast_fd);

  m_mcast_fd = kInvalidSocket;
  m_ucast_fd = kInvalidSocket;

  return RT_OK;
}

rtError
rtRemoteMulticastResolver::registerObject(std::string const& name, sockaddr_storage const& endpoint)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_hosted_objects[name] = endpoint;
  lock.unlock(); // TODO this wasn't here before.  Make sure it's right to put it here
  return RT_OK;
}

rtError
rtRemoteMulticastResolver::unregisterObject(std::string const& name)
{
  rtError e = RT_OK;

  std::unique_lock<std::mutex> lock(m_mutex);
  auto itr = m_hosted_objects.find(name);
  if (itr != m_hosted_objects.end())
  {
    m_hosted_objects.erase(itr);
    e = RT_OK;
  }
  else
  {
    e = RT_ERROR_OBJECT_NOT_FOUND;
  }
  return e;
}
