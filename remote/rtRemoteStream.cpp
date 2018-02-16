#include "rtRemoteStream.h"
#include "rtRemoteMessage.h"
#include "rtRemoteConfig.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteStreamSelector.h"

#include <algorithm>
#include <memory>
#include <thread>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <rtLog.h>

rtRemoteStream::rtRemoteStream(rtRemoteEnvironment* env, int fd, sockaddr_storage const& local_endpoint,
  sockaddr_storage const& remote_endpoint)
  : m_fd(fd)
  , m_env(env)
{
  memcpy(&m_remote_endpoint, &remote_endpoint, sizeof(m_remote_endpoint));
  memcpy(&m_local_endpoint, &local_endpoint, sizeof(m_local_endpoint));
}

rtRemoteStream::~rtRemoteStream()
{
  this->close();
}

rtError
rtRemoteStream::open()
{
  auto self = shared_from_this();
  m_env->StreamSelector->registerStream(self);
  return RT_OK;
}

rtError
rtRemoteStream::close()
{
  if (m_fd != kInvalidSocket)
  {
    // rtRemoteStreamSelector will remove dead streams on its own
    int ret = ::shutdown(m_fd, SHUT_RDWR);
    if (NET_FAILED(ret))
    {
      rtError e = rtErrorFromErrno(net_errno());
      rtLogDebug("shutdown failed on fd %d: %s", m_fd, rtStrError(e));
    }

    // sets m_fd to kInvalidSocket
    rtCloseSocket(m_fd);
  }

  return RT_OK;
}

rtError
rtRemoteStream::connect()
{
  RT_ASSERT(m_fd == kInvalidSocket);
  return connectTo(m_remote_endpoint);
}

rtError
rtRemoteStream::connectTo(sockaddr_storage const& endpoint)
{
  m_fd = socket(endpoint.ss_family, SOCK_STREAM, 0);
  if (NET_FAILED(m_fd))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to create socket. %s", rtStrError(e));
    return e;
  }

  if (endpoint.ss_family != AF_UNIX)
  {
    uint32_t one = 1;
    if (NET_FAILED(setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one))))
      rtLogError("setting TCP_NODELAY failed");
  }

  socklen_t len;
  rtSocketGetLength(endpoint, &len);

  int ret = ::connect(m_fd, reinterpret_cast<sockaddr const *>(&endpoint), len);
  if (NET_FAILED(ret))
  {
    rtError e = rtErrorFromErrno(net_errno());
    rtLogError("failed to connect to remote rpc endpoint. %s", rtStrError(e));
    rtCloseSocket(m_fd);
    return e;
  }

  rtGetSockName(m_fd, m_local_endpoint);
  rtGetPeerName(m_fd, m_remote_endpoint);

  rtLogInfo("new connection (%d) %s --> %s",
    m_fd,
    rtSocketToString(m_local_endpoint).c_str(),
    rtSocketToString(m_remote_endpoint).c_str());

  return RT_OK;
}

rtError
rtRemoteStream::send(rtRemoteMessagePtr const& msg)
{
  return rtSendDocument(*msg, m_fd, nullptr);
}

rtRemoteAsyncHandle
rtRemoteStream::sendWithWait(rtRemoteMessagePtr const& msg, rtRemoteCorrelationKey k)
{
  rtRemoteAsyncHandle asyncHandle(m_env, k);
  rtError e = rtSendDocument(*msg, m_fd, nullptr);
  if (e != RT_OK)
    asyncHandle.complete(rtRemoteMessagePtr(), e);
  return asyncHandle;
}

rtError
rtRemoteStream::onInactivity()
{
  std::shared_ptr<CallbackHandler> handler = m_callback_handler.lock();
  if (handler)
  {
    auto self = shared_from_this();
    rtError e = handler->onStateChanged(self, State::Inactive);
    if (e != RT_OK)
      rtLogError("failed dispatching inactivity handler. %s", rtStrError(e));
  }
  return RT_OK;
}


rtError
rtRemoteStream::onIncomingMessage(rtRemoteSocketBuffer& buff)
{
  std::shared_ptr<CallbackHandler> handler = m_callback_handler.lock();

  rtRemoteMessagePtr doc = nullptr;
  rtError e = rtReadMessage(m_fd, buff, doc);
  if (e != RT_OK)
  {
#ifdef RT_PLATFORM_WINDOWS
     if ((e == rtErrorFromErrno(ENOTCONN) || 
        e == rtErrorFromErrno(WSAENOTCONN) ||
        e == rtErrorFromErrno(WSAECONNRESET))
        && handler)
#else
    if (e == rtErrorFromErrno(ENOTCONN) && handler)
#endif
    { 
      auto self = shared_from_this();
      rtError err = handler->onStateChanged(self, State::Closed);
      if (err != RT_OK)
        rtLogWarn("failed to invoke state changed handler. %s", rtStrError(err));

      // return the error back to the caller so they know that that stream is dead
      return e;
    }
    rtLogDebug("failed to read message. %s", rtStrError(e));
  }

  if (e == RT_OK && handler)
    e = handler->onMessage(doc);

  return RT_OK;
}

rtError
rtRemoteStream::setCallbackHandler(std::shared_ptr<CallbackHandler> const& handler)
{
  if (!handler)
    return RT_ERROR_INVALID_ARG;

  m_callback_handler = handler;
  return RT_OK;
}
