#include "rtRemoteStreamSelector.h"
#include "rtRemoteConfig.h"
#include "rtRemoteStream.h"
#include "rtRemoteSocketUtils.h"
#include "rtError.h"
#include "rtLog.h"

#include <algorithm>
#include <chrono>
#include <fcntl.h>

rtRemoteStreamSelector::rtRemoteStreamSelector(rtRemoteEnvironment* env)
  : m_env(env)
  , m_shutdown(false)
  , m_running(false)
{
}

void*
rtRemoteStreamSelector::pollFds(void* argp)
{
  rtRemoteStreamSelector* selector = reinterpret_cast<rtRemoteStreamSelector *>(argp);
  rtError e = selector->doPollFds();
  if (e != RT_OK)
    rtLogInfo("pollFds error. %s", rtStrError(e));
  return nullptr;
}

rtError
rtRemoteStreamSelector::start()
{
  m_running = true;
  rtLogInfo("starting StreamSelector");
  m_thread.reset(new std::thread(&rtRemoteStreamSelector::pollFds, this));
  return RT_OK;
}

rtError
rtRemoteStreamSelector::registerStream(std::shared_ptr<rtRemoteStream> const& s)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_streams.push_back(s);
  m_streams_cond.notify_all();
  return RT_OK;
}

rtError
rtRemoteStreamSelector::shutdown()
{
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_running = false;
    m_streams_cond.notify_all();
  }


  rtLogInfo("sending shutdown signal");
  m_shutdown = true;

  if (m_thread)
   m_thread->join();

  return RT_OK;
}

rtError
rtRemoteStreamSelector::doPollFds()
{
  rtRemoteSocketBuffer buff;
  buff.reserve(m_env->Config->stream_socket_buffer_size());
  buff.resize(m_env->Config->stream_socket_buffer_size());

  const auto keepAliveInterval = std::chrono::seconds(m_env->Config->stream_keep_alive_interval());
  auto lastKeepAliveSent = std::chrono::steady_clock::now();

  while (true)
  {
    int maxFd = 0;

    fd_set readFds;
    fd_set errFds;

    FD_ZERO(&readFds);
    FD_ZERO(&errFds);

    {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_streams_cond.wait(lock, [&]() {return !m_streams.empty() || !m_running;});

    if (!m_running)
      return RT_OK;

    // remove dead streams
    {
      auto itr = std::remove_if(m_streams.begin(), m_streams.end(),
          [](std::shared_ptr<rtRemoteStream> const& s)
          {
            return !s->isOpen();
          });

      if (itr != m_streams.end())
        m_streams.erase(itr);
    }

    for (auto const& s : m_streams)
    {
      rtPushFd(&readFds, s->m_fd, &maxFd);
      rtPushFd(&errFds, s->m_fd, &maxFd);
    }
    }

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    int ret = select(maxFd + 1, &readFds, NULL, &errFds, &timeout);
    if (NET_FAILED(ret))
    {
      rtError e = rtErrorFromErrno(net_errno());
      rtLogWarn("select failed: %s", rtStrError(e));
      continue;
    }

    if (m_shutdown)
    {
      rtLogInfo("got shutdown signal");
      return RT_OK;
    }

    auto now = std::chrono::steady_clock::now();
    bool sentKeepAlive = false;

    std::unique_lock<std::mutex> lock(m_mutex);
    for (int i = 0, n = static_cast<int>(m_streams.size()); i < n; ++i)
    {
      rtError e = RT_OK;
      std::shared_ptr<rtRemoteStream> s = m_streams[i];
      if (FD_ISSET(s->m_fd, &readFds))
      {
        e = s->onIncomingMessage(buff);
        if (e != RT_OK)
        {
          rtLogWarn("error dispatching message. %s", rtStrError(e));
          m_streams[i].reset();
          s.reset();
        }
      }
      else if (FD_ISSET(s->m_fd, &errFds))
      {
        // TODO
        rtLogError("error on fd: %d", s->m_fd);
      }

      if (s && s->isOpen() && (now - lastKeepAliveSent) > keepAliveInterval)
      {
        sentKeepAlive = true;

        // This really isn't inactivity, it's more like a timer enve
        e = s->onInactivity();
        if (e != RT_OK)
          rtLogWarn("error sending keep alive. %s", rtStrError(e));
      }
    }

    if (sentKeepAlive)
    {
      lastKeepAliveSent = now;
    }

    // remove all dead streams
    auto end = std::remove_if(m_streams.begin(), m_streams.end(),
        [](std::shared_ptr<rtRemoteStream> const& s) { return s == nullptr; });
    m_streams.erase(end, m_streams.end());
    // rtLogDebug("streams size:%d", (int) m_streams.size());
    lock.unlock();
  }

  return RT_OK;
}
