#include "rtRemoteResolver.h"
#include "rtSocketUtils.h"
#include "rtRemoteMessage.h"
#include "rtRemoteConfig.h"

#include <condition_variable>
#include <thread>
#include <mutex>

#include <rtLog.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

class rtRemoteMulticastResolver : public rtIRpcResolver
{
public:
  rtRemoteMulticastResolver();
  ~rtRemoteMulticastResolver();

public:
  virtual rtError open(sockaddr_storage const& rpc_endpoint) override;
  virtual rtError close() override;
  virtual rtError registerObject(std::string const& name, sockaddr_storage const& endpoint) override;
  virtual rtError locateObject(std::string const& name, sockaddr_storage& endpoint,
    uint32_t timeout) override;

private:
  using CommandHandler = rtError (rtRemoteMulticastResolver::*)(rtJsonDocPtr const&, sockaddr_storage const&);
  using HostedObjectsMap = std::map< std::string, sockaddr_storage >;
  using CommandHandlerMap = std::map< std::string, CommandHandler >;
  using RequestMap = std::map< rtCorrelationKey, rtJsonDocPtr >;

  void runListener();
  void doRead(int fd, rtSocketBuffer& buff);
  void doDispatch(char const* buff, int n, sockaddr_storage* peer);

  rtError init();
  rtError openUnicastSocket();
  rtError openMulticastSocket();

  // command handlers
  rtError onSearch(rtJsonDocPtr const& doc, sockaddr_storage const& soc);
  rtError onLocate(rtJsonDocPtr const& doc, sockaddr_storage const& soc);

private:
  sockaddr_storage  m_mcast_dest;
  sockaddr_storage  m_mcast_src;
  int               m_mcast_fd;
  uint32_t          m_mcast_src_index;

  sockaddr_storage  m_ucast_endpoint;
  int               m_ucast_fd;
  socklen_t         m_ucast_len;

  std::unique_ptr<std::thread> m_read_thread;
  std::condition_variable m_cond;
  std::mutex        m_mutex;
  pid_t             m_pid;
  CommandHandlerMap m_command_handlers;
  std::string       m_rpc_addr;
  uint16_t          m_rpc_port;
  HostedObjectsMap  m_hosted_objects;
  RequestMap	    m_pending_searches;
  int		    m_shutdown_pipe[2];
};

rtRemoteMulticastResolver::rtRemoteMulticastResolver()
  : m_mcast_fd(-1)
  , m_mcast_src_index(-1)
  , m_ucast_fd(-1)
  , m_ucast_len(0)
  , m_pid(getpid())
  , m_command_handlers()
{
  memset(&m_mcast_dest, 0, sizeof(m_mcast_dest));
  memset(&m_mcast_src, 0, sizeof(m_mcast_src));
  memset(&m_ucast_endpoint, 0, sizeof(m_ucast_endpoint));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeSearch, &rtRemoteMulticastResolver::onSearch));
  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeLocate, &rtRemoteMulticastResolver::onLocate));

  m_shutdown_pipe[0] = -1;
  m_shutdown_pipe[1] = -1;

  int ret = pipe2(m_shutdown_pipe, O_CLOEXEC);
  if (ret == -1)
    rtLogWarn("failed to create shutdown pipe. %s", rtStrError(ret).c_str());
}

rtRemoteMulticastResolver::~rtRemoteMulticastResolver()
{
  rtError err = this->close();
  if (err != RT_OK)
    rtLogWarn("failed to close resolver: %s", rtStrError(err));
}

rtError
rtRemoteMulticastResolver::init()
{
  rtError err = RT_OK;

  uint16_t port = rtRemoteSetting<uint16_t>("rt.rpc.resolver.multicast_port");
  char const* dstaddr = rtRemoteSetting<char const *>("rt.rpc.resolver.multicast_address");

  err = rtParseAddress(m_mcast_dest, dstaddr, port, nullptr);
  if (err != RT_OK)
  {
    rtLogWarn("failed to parse address: %s. %s", dstaddr, rtStrError(err));
    return err;
  }

  char const* srcaddr = rtRemoteSetting<char const *>("rt.rpc.resolver.multicast_interface");

  err = rtParseAddress(m_mcast_src, srcaddr, port, &m_mcast_src_index);
  if (err != RT_OK)
  {
    err = rtGetDefaultInterface(m_mcast_src, port);
    if (err != RT_OK)
      return err;
  }

  err = rtParseAddress(m_ucast_endpoint, srcaddr, 0, nullptr);
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
    char buff[128];

    void* addr = nullptr;
    rtGetInetAddr(rpc_endpoint, &addr);

    socklen_t len;
    rtSocketGetLength(rpc_endpoint, &len);
    char const* p = inet_ntop(rpc_endpoint.ss_family, addr, buff, len);
    if (p)
      m_rpc_addr = p;

    rtGetPort(rpc_endpoint, &m_rpc_port);
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

  m_read_thread.reset(new std::thread(&rtRemoteMulticastResolver::runListener, this));
  return RT_OK;
}

rtError
rtRemoteMulticastResolver::openUnicastSocket()
{
  int ret = 0;
  int err = 0;

  const int socket_type = SOCK_DGRAM;
  m_ucast_fd = socket(m_ucast_endpoint.ss_family, socket_type, 0);

  if (m_ucast_fd < 0)
  {
    err = errno;
    rtLogError("failed to create unicast socket with family: %d. %s", 
      m_ucast_endpoint.ss_family, rtStrError(errno).c_str());
    return RT_FAIL;
  }

  if (rtSocketSetNoDelay(m_ucast_fd, socket_type) != RT_OK)
    rtLogError("setting TCP_NODELAY failed");

  fcntl(m_ucast_fd, F_SETFD, fcntl(m_ucast_fd, F_GETFD) | FD_CLOEXEC);

  rtSocketGetLength(m_ucast_endpoint, &m_ucast_len);

  // listen on ANY port
  ret = bind(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), m_ucast_len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to bind unicast endpoint: %s", rtStrError(errno).c_str());
    return RT_FAIL;
  }

  // now figure out which port we're bound to
  rtSocketGetLength(m_ucast_endpoint, &m_ucast_len);
  ret = getsockname(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), &m_ucast_len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to get socketname. %s", rtStrError(errno).c_str());
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
    rtLogError("failed to set outgoing multicast interface. %s", rtStrError(errno).c_str());
    return RT_FAIL;
  }

  #if 0
  char loop = 1;
  if (setsockopt(m_ucast_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) < 0)
  {
    rtLogError("failed to disable multicast loopback: %s", rtStrError(errno).c_str());
    return RT_FAIL;
  }
  #endif

  return RT_OK;
}

rtError
rtRemoteMulticastResolver::onSearch(rtJsonDocPtr const& doc, sockaddr_storage const& soc)
{
  auto senderId = doc->FindMember(kFieldNameSenderId);
  assert(senderId != doc->MemberEnd());
  if (senderId->value.GetInt() == m_pid)
  {
    return RT_OK;
  }

  int key = rtMessage_GetCorrelationKey(*doc);


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
    doc.AddMember(kFieldNameIp, m_rpc_addr, doc.GetAllocator());
    doc.AddMember(kFieldNamePort, m_rpc_port, doc.GetAllocator());
    // echo kback to sender
    doc.AddMember(kFieldNameSenderId, senderId->value.GetInt(), doc.GetAllocator());
    doc.AddMember(kFieldNameCorrelationKey, key, doc.GetAllocator());

    return rtSendDocument(doc, m_ucast_fd, &soc);
  }

  return RT_OK;
}

rtError
rtRemoteMulticastResolver::onLocate(rtJsonDocPtr const& doc, sockaddr_storage const& /*soc*/)
{
  int key = rtMessage_GetCorrelationKey(*doc);

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

  rtError err = RT_OK;
  rtCorrelationKey seqId = rtMessage_GetNextCorrelationKey();

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember(kFieldNameMessageType, kMessageTypeSearch, doc.GetAllocator());
  doc.AddMember(kFieldNameObjectId, name, doc.GetAllocator());
  doc.AddMember(kFieldNameSenderId, m_pid, doc.GetAllocator());
  doc.AddMember(kFieldNameCorrelationKey, seqId, doc.GetAllocator());

  err = rtSendDocument(doc, m_ucast_fd, &m_mcast_dest);
  if (err != RT_OK)
    return err;

  rtJsonDocPtr searchResponse;
  RequestMap::const_iterator itr;

  auto delay = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);

  // wait here until timeout expires or we get a response that matches out pid/seqid
  std::unique_lock<std::mutex> lock(m_mutex);
  itr = m_pending_searches.end();
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
    assert(searchResponse->HasMember(kFieldNameIp));
    assert(searchResponse->HasMember(kFieldNamePort));

    rtError err = rtParseAddress(endpoint, (*searchResponse)[kFieldNameIp].GetString(),
        (*searchResponse)[kFieldNamePort].GetInt(), nullptr);

    if (err != RT_OK)
      return err;
  }

  return RT_OK;
}

void
rtRemoteMulticastResolver::runListener()
{
  rtSocketBuffer buff;
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
    rtPushFd(&read_fds, m_shutdown_pipe[0], &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_mcast_fd, &maxFd);
    rtPushFd(&err_fds, m_ucast_fd, &maxFd);

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, NULL);
    if (ret == -1)
    {
      rtLogWarn("select failed: %s", rtStrError(errno).c_str());
      continue;
    }

    if (FD_ISSET(m_shutdown_pipe[0], &read_fds))
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
rtRemoteMulticastResolver::doRead(int fd, rtSocketBuffer& buff)
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
rtRemoteMulticastResolver::doDispatch(char const* buff, int n, sockaddr_storage* peer)
{
  // rtLogInfo("new message from %s:%d", inet_ntoa(src.sin_addr), htons(src.sin_port));
  // printf("read: %d\n", int(n));
  #ifdef RT_RPC_DEBUG
  rtLogDebug("read:\n***IN***\t\"%.*s\"\n", n, buff); // static_cast<int>(m_read_buff.size()), &m_read_buff[0]);
  #endif

  rtJsonDocPtr doc;
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
  if (m_shutdown_pipe[1] != -1)
  {
    char buff[] = {"shutdown"};
    write(m_shutdown_pipe[1], buff, sizeof(buff));

    if (m_read_thread)
    {
      m_read_thread->join();
      m_read_thread.reset();
    }

    if (m_shutdown_pipe[0] != -1)
      ::close(m_shutdown_pipe[0]);
    if (m_shutdown_pipe[1] != -1)
      ::close(m_shutdown_pipe[1]);
  }

  if (m_mcast_fd != -1)
    ::close(m_mcast_fd);
  if (m_ucast_fd != -1)
    ::close(m_ucast_fd);

  m_mcast_fd = -1;
  m_ucast_fd = -1;
  m_shutdown_pipe[0] = -1;
  m_shutdown_pipe[1] = -1;

  return RT_OK;
}

rtError
rtRemoteMulticastResolver::registerObject(std::string const& name, sockaddr_storage const& endpoint)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_hosted_objects[name] = endpoint;
  return RT_OK;
}

rtError
rtRemoteMulticastResolver::openMulticastSocket()
{
  int err = 0;
  
  const int socket_type = SOCK_DGRAM;
  m_mcast_fd = socket(m_mcast_dest.ss_family, socket_type, 0);

  if (m_mcast_fd < 0)
  {
    err = errno;
    rtLogError("failed to create datagram socket with family:%d. %s",
      m_mcast_dest.ss_family, rtStrError(errno).c_str());
    return RT_FAIL;
  }
  if (rtSocketSetNoDelay(m_mcast_fd, socket_type) != RT_OK)
    rtLogError("setting TCP_NODELAY failed");

  fcntl(m_mcast_fd, F_SETFD, fcntl(m_mcast_fd, F_GETFD) | FD_CLOEXEC);

  // re-use because multiple applications may want to join group on same machine
  int reuse = 1;
  err = setsockopt(m_mcast_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to set reuseaddr. %s", rtStrError(errno).c_str());
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
        rtSocketToString(m_mcast_src).c_str(),  rtStrError(errno).c_str());
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
      rtStrError(errno).c_str());
    return RT_FAIL;
  }

  rtLogInfo("successfully joined multicast group: %s on interface: %s",
    rtSocketToString(m_mcast_dest).c_str(), rtSocketToString(m_mcast_src).c_str());

  return RT_OK;
}

rtIRpcResolver* rtRemoteCreateResolver()
{
  return new rtRemoteMulticastResolver();
}
