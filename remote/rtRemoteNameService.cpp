#include "rtRemoteNameService.h"
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

rtRemoteNameService::rtRemoteNameService(rtRemoteEnvironment* env)
  : m_ns_fd(-1)
  , m_ns_len(0)
  , m_pid(getpid())
  , m_command_handlers()
  , m_shutdown(false)
  , m_env(env)
{
  memset(&m_ns_endpoint, 0, sizeof(m_ns_endpoint));

  m_command_handlers.insert(CommandHandlerMap::value_type(kNsMessageTypeRegister, &rtRemoteNameService::onRegister));
  m_command_handlers.insert(CommandHandlerMap::value_type(kNsMessageTypeDeregister, &rtRemoteNameService::onDeregister));
  m_command_handlers.insert(CommandHandlerMap::value_type(kNsMessageTypeUpdate, &rtRemoteNameService::onUpdate));
  m_command_handlers.insert(CommandHandlerMap::value_type(kNsMessageTypeLookup, &rtRemoteNameService::onLookup));
}

rtRemoteNameService::~rtRemoteNameService()
{
  rtError err = this->close();
  if (err != RT_OK)
    rtLogWarn("failed to close resolver: %s", rtStrError(err));
}

rtError
rtRemoteNameService::init()
{
  rtError err = RT_OK;

  // TODO eventually, use a db rather than map.  Open it here.

  // get socket info ready
  uint16_t const nsport = m_env->Config->resolver_unicast_port();
  std::string nsaddr = m_env->Config->resolver_unicast_address();
  err = rtParseAddress(m_ns_endpoint, nsaddr.c_str(), nsport, nullptr);
  if (err != RT_OK)
  {
    err = rtGetDefaultInterface(m_ns_endpoint, 0);
    if (err != RT_OK)
      return err;
  }

  // open unicast socket
  err = openNsSocket();
  if (err != RT_OK)
  {
      rtLogWarn("failed to open name service unicast socket. %s", rtStrError(err));
      return err;
  }

  m_shutdown = false;

  m_read_thread.reset(new std::thread(&rtRemoteNameService::runListener, this));
  return err;
}

rtError
rtRemoteNameService::close()
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

  if (m_ns_fd != kInvalidSocket)
    rtCloseSocket(m_ns_fd);

  m_ns_fd = kInvalidSocket;

  return RT_OK;
}

// rtError 
// rtRemoteNameService::openDbConnection(){}

rtError
rtRemoteNameService::openNsSocket()
{
  int ret = 0;

  m_ns_fd = socket(m_ns_endpoint.ss_family, SOCK_DGRAM, 0);
  if (NET_FAILED(m_ns_fd))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to create unicast socket with family: %d. %s", 
      m_ns_endpoint.ss_family, rtStrError(e));
    return e;
  }

  rtSocketGetLength(m_ns_endpoint, &m_ns_len);

  // listen on ANY port
  ret = bind(m_ns_fd, reinterpret_cast<sockaddr *>(&m_ns_endpoint), m_ns_len);
  if (NET_FAILED(ret))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to bind unicast endpoint: %s", rtStrError(e));
    return e;
  }

  // now figure out which port we're bound to
  rtSocketGetLength(m_ns_endpoint, &m_ns_len);
  ret = getsockname(m_ns_fd, reinterpret_cast<sockaddr *>(&m_ns_endpoint), &m_ns_len);
  if (NET_FAILED(ret))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to get socketname. %s", rtStrError(e));
    return e;
  }
  else
  {
    rtLogInfo("local udp socket bound to %s", rtSocketToString(m_ns_endpoint).c_str());
  }

  return RT_OK;
}

/**
 * Callback for registering objects and associated Well-known Endpoints
 */
rtError
rtRemoteNameService::onRegister(rtRemoteMessagePtr const& doc, sockaddr_storage const& /*soc*/)
{
  RT_ASSERT(doc->HasMember(kFieldNameIp));
  RT_ASSERT(doc->HasMember(kFieldNamePort));
  
  sockaddr_storage endpoint;
  rtError err = rtParseAddress(endpoint, (*doc)[kFieldNameIp].GetString(),
                (*doc)[kFieldNamePort].GetInt(), nullptr);

  if (err != RT_OK)
    return err;
  
  char const* objectId = rtMessage_GetObjectId(*doc);
  
  std::unique_lock<std::mutex> lock(m_mutex);
  m_registered_objects[objectId] = endpoint;
  lock.unlock();
  return RT_OK;
}

/**
 * Callback for deregistering objects
 */
rtError
rtRemoteNameService::onDeregister(rtRemoteMessagePtr const& /*doc*/, sockaddr_storage const& /*soc*/)
{
  // TODO
  return RT_OK;
}

rtError
rtRemoteNameService::onUpdate(rtRemoteMessagePtr const& /*doc*/, sockaddr_storage const& /*soc*/)
{
  // TODO
  return RT_OK;
}

rtError
rtRemoteNameService::onLookup(rtRemoteMessagePtr const& doc, sockaddr_storage const& soc)
{
  auto senderId = doc->FindMember(kFieldNameSenderId);
  RT_ASSERT(senderId != doc->MemberEnd());
  if (senderId->value.GetInt() == m_pid)
    return RT_OK;

  rtRemoteCorrelationKey key = rtMessage_GetCorrelationKey(*doc);

  auto itr = m_registered_objects.end();

  char const* objectId = rtMessage_GetObjectId(*doc);

  std::unique_lock<std::mutex> lock(m_mutex);
  itr = m_registered_objects.find(objectId);
  lock.unlock();

  if (itr != m_registered_objects.end())
  { // object is registered

    // get IP and port
    sockaddr_storage endpoint = itr->second;
    std::string       ep_addr;
    uint16_t          ep_port;
    char buff[128];

    void* addr = nullptr;
    rtGetInetAddr(endpoint, &addr);

    socklen_t len;
    rtSocketGetLength(endpoint, &len);
    char const* p = inet_ntop(endpoint.ss_family, addr, buff, len);
    if (p)
      ep_addr = p;
    rtGetPort(endpoint, &ep_port);

    // create and send response
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember(kFieldNameMessageType, kNsMessageTypeLookupResponse, doc.GetAllocator());
    doc.AddMember(kFieldNameStatusMessage, kNsStatusSuccess, doc.GetAllocator());
    doc.AddMember(kFieldNameObjectId, std::string(objectId), doc.GetAllocator());
    doc.AddMember(kFieldNameIp, ep_addr, doc.GetAllocator());
    doc.AddMember(kFieldNamePort, ep_port, doc.GetAllocator());
    doc.AddMember(kFieldNameSenderId, senderId->value.GetInt(), doc.GetAllocator());
    doc.AddMember(kFieldNameCorrelationKey, key.toString(), doc.GetAllocator());

    return rtSendDocument(doc, m_ns_fd, &soc);
  }
  return RT_OK;
}

void
rtRemoteNameService::runListener()
{
  rtRemoteSocketBuffer buff;
  buff.reserve(1024 * 1024);
  buff.resize(1024 * 1024);

  while (true)
  {
    rtLogInfo("listening");
    int maxFd = 0;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    rtPushFd(&read_fds, m_ns_fd, &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_ns_fd, &maxFd);

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

    if (FD_ISSET(m_ns_fd, &read_fds))
      doRead(m_ns_fd, buff);
  }
}

void
rtRemoteNameService::doRead(socket_t fd, rtRemoteSocketBuffer& buff)
{
  rtLogInfo("doing read");
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
rtRemoteNameService::doDispatch(char const* buff, int n, sockaddr_storage* peer)
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
