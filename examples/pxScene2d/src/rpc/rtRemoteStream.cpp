#include "rtRemoteStream.h"
#include "rtRemoteMessage.h"
#include "rtRemoteConfig.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteStreamSelector.h"

#include <algorithm>
#include <memory>
#include <thread>
#include <vector>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <rtLog.h>

rtRemoteStream::rtRemoteStream(rtRemoteEnvironment* env, int fd, sockaddr_storage const& local_endpoint,
  sockaddr_storage const& remote_endpoint)
  : m_fd(fd)
  , m_last_message_time(0)
  , m_last_ka_message_time(0)
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
    int ret = 0;
    
    ret = ::shutdown(m_fd, SHUT_RDWR);
    if (ret == -1)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogDebug("shutdown failed on fd %d: %s", m_fd, rtStrError(e));
    }

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
  if (m_fd < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to create socket. %s", rtStrError(e));
    return e;
  }
  fcntl(m_fd, F_SETFD, fcntl(m_fd, F_GETFD) | FD_CLOEXEC);

  if (endpoint.ss_family != AF_UNIX)
  {
    uint32_t one = 1;
    if (-1 == setsockopt(m_fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one)))
      rtLogError("setting TCP_NODELAY failed");
  }

  socklen_t len;
  rtSocketGetLength(endpoint, &len);

  int ret = ::connect(m_fd, reinterpret_cast<sockaddr const *>(&endpoint), len);
  if (ret < 0)
  {
    rtError e = rtErrorFromErrno(errno);
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
  m_last_message_time = time(0);
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
rtRemoteStream::setStateChangedHandler(StateChangedHandler handler, void* argp)
{
  m_state_changed_handler.Func = handler;
  m_state_changed_handler.Arg = argp;
  return RT_OK;
}

rtError
rtRemoteStream::setMessageHandler(MessageHandler handler, void* argp)
{
  m_message_handler.Func = handler;
  m_message_handler.Arg = argp;
  return RT_OK;
}

rtError
rtRemoteStream::onIncomingMessage(rtSocketBuffer& buff, time_t now)
{
  rtRemoteMessagePtr doc = nullptr;
  m_last_message_time = now;
  rtError e = rtReadMessage(m_fd, buff, doc);
  if (e != RT_OK)
  {
    if (e == rtErrorFromErrno(ENOTCONN) && m_state_changed_handler.Func)
    { 
      auto self = shared_from_this();
      rtError err = m_state_changed_handler.Func(self, State::Closed, m_state_changed_handler.Arg);
      if (err != RT_OK)
        rtLogWarn("failed to invoke state changed handler. %s", rtStrError(err));

      // return the error back to the caller so they know that that stream is dead
      return e;
    }
    rtLogDebug("failed to read message. %s", rtStrError(e));
  }

  if (e == RT_OK && m_message_handler.Func != nullptr)
    e = m_message_handler.Func(doc, m_message_handler.Arg);

  return RT_OK;
}

#if 0
rtError
rtRemoteStream::sendRequest2(rtRemoteRequest const& req, MessageHandler handler, void* argp)
{
  rtRemoteCorrelationKey key = req.getCorrelationKey();
  RT_ASSERT(key != 0);
  RT_ASSERT(m_fd != kInvalidSocket);

  m_last_message_time = time(0);
  m_send_mutex.lock();
  rtError e = req.send(m_fd, NULL);
  m_send_mutex.unlock();

  if (e != RT_OK)
  {
    rtLogWarn("failed to send request: %s", rtStrError(e));
    return e;
  }

  if (handler)
  {
    Callback<MessageHandler> callback;
    callback.Func = handler;
    callback.Arg = argp;

    std::unique_lock<std::mutex> lock(m_mutex);
    m_callbacks.insert(CallbackMap::value_type(key, callback));
  }

  return RT_OK;
}
#endif

#if 0
rtRemoteMessagePtr
rtRemoteStream::waitForResponse(rtRemoteCorrelationKey key, uint32_t timeout)
{
  rtRemoteMessagePtr res = nullptr;

  rtLogInfo("waiting for: %d", (int) key);

  if (timeout == 0)
    timeout = m_env->Config->environment_request_timeout();

  std::chrono::system_clock::time_point delay =
    std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);

  // if we're using internally dispatched threads, then things will arrive here from the
  // background thread. Let's wait until we find our own response, otherwise, worker
  // threads will process these items.
  if (m_env->Config->server_use_dispatch_thread())
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait_until(lock, delay, [this, key, &res] { return (res = getResponse(key)) != nullptr; });
    lock.unlock();
    return res;
  }
  else
  {
    // if we're the thread responsible for processing items in our queue, then
    // we have to sort of preempt ourselves for incoming messages and process them,
    // while also honoring the timeout.
    while (delay > std::chrono::system_clock::now())
    {
      // wait for ANY incoming message. This is different that the previous
      // branch where we wait for the response to the current 'key' in context
      std::unique_lock<std::mutex> lock(m_mutex);
      // delay is wall clock, not relative, so no need to update on each iteration
      m_cond.wait_until(lock, delay, [this, key, &res] { !m_queue.empty(); });
      lock.unlock();

      return res;
    }
  }

  return rtRemoteMessagePtr();
}
#endif
