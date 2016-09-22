#include "rtRemoteStreamSelector.h"
#include "rtRemoteStream.h"
#include "rtRemoteSocketUtils.h"
#include "rtError.h"
#include "rtLog.h"

#include <algorithm>
#include <unistd.h>
#include <fcntl.h>

rtRemoteStreamSelector::rtRemoteStreamSelector()
{
  int ret = pipe2(m_shutdown_pipe, O_CLOEXEC);
  if (ret == -1)
  {
    rtError e = rtErrorFromErrno(ret);
    rtLogError("failed to create pipe. %s", rtStrError(e));
  }
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
  rtLogInfo("starting StreamSelector");
  pthread_create(&m_thread, nullptr, &rtRemoteStreamSelector::pollFds, this);
  return RT_OK;
}

rtError
rtRemoteStreamSelector::registerStream(std::shared_ptr<rtRemoteStream> const& s)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_streams.push_back(s);
  return RT_OK;
}

rtError
rtRemoteStreamSelector::removeStream(std::shared_ptr<rtRemoteStream> const& stream)
{
  rtError e = RT_ERROR_OBJECT_NOT_FOUND;

  std::unique_lock<std::mutex> lock(m_mutex);
  auto itr = std::remove_if(
    m_streams.begin(),
    m_streams.end(),
    [&stream](std::shared_ptr<rtRemoteStream> const& s) { return s.get() == stream.get(); });
  if (itr != m_streams.end())
  {
    m_streams.erase(itr);
    e = RT_OK;
  }

  return e;
}

rtError
rtRemoteStreamSelector::shutdown()
{
  char buff[] = { "shudown" };
  rtLogInfo("sending shutdown signal");
  ssize_t n = write(m_shutdown_pipe[1], buff, sizeof(buff));
  if (n == -1)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogWarn("failed to write. %s", rtStrError(e));
  }

  void* retval = nullptr;
  pthread_join(m_thread, &retval);

  ::close(m_shutdown_pipe[0]);
  ::close(m_shutdown_pipe[1]);

  return RT_OK;
}

rtError
rtRemoteStreamSelector::doPollFds()
{
  rtRemoteSocketBuffer buff;
  buff.reserve(1024 * 1024);
  buff.resize(1024 * 1024);

  while (true)
  {
    int maxFd = 0;

    fd_set readFds;
    fd_set errFds;

    FD_ZERO(&readFds);
    FD_ZERO(&errFds);

    for (auto const& s : m_streams)
    {
      rtPushFd(&readFds, s->m_fd, &maxFd);
      rtPushFd(&errFds, s->m_fd, &maxFd);
    }
    rtPushFd(&readFds, m_shutdown_pipe[0], &maxFd);

    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int ret = select(maxFd + 1, &readFds, NULL, &errFds, &timeout);
    if (ret == -1)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogWarn("select failed: %s", rtStrError(e));
      continue;
    }

    if (FD_ISSET(m_shutdown_pipe[0], &readFds))
    {
      rtLogInfo("got shutdown signal");
      return RT_OK;
    }

    time_t now = time(0);

    std::unique_lock<std::mutex> lock(m_mutex);
    for (int i = 0, n = static_cast<int>(m_streams.size()); i < n; ++i)
    {
      // TODO: make sure stream is still registerd

      rtError e = RT_OK;
      std::shared_ptr<rtRemoteStream> const& s = m_streams[i];
      if (FD_ISSET(s->m_fd, &readFds))
      {
        e = s->onIncomingMessage(buff, now);
        if (e != RT_OK)
        {
          rtLogWarn("error dispatching message. %s", rtStrError(e));
          m_streams[i].reset();
        }
      }


      #if 0
      if (s && (now - s->m_last_ka_message_time > 10))
      {
        s->m_last_ka_message_time = time(0);
        e = s->onInactivity(now);
        if (e != RT_OK)
          m_streams[i].reset();
      }
      #endif
    }

    // remove all dead streams
    auto end = std::remove_if(m_streams.begin(), m_streams.end(),
        [](std::shared_ptr<rtRemoteStream> const& s) { return s == nullptr; });
    m_streams.erase(end, m_streams.end());
    rtLogDebug("streams size:%d", (int) m_streams.size());
    lock.unlock();
  }

  return RT_OK;
}
