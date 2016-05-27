#include "rtRemoteStream.h"
#include "rtRemoteMessage.h"

#include <algorithm>
#include <memory>
#include <thread>
#include <vector>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <rtLog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

class rtRemoteStreamSelector
{
public:
  rtRemoteStreamSelector()
  {
    int ret = pipe2(m_shutdown_pipe, O_CLOEXEC);
    if (ret == -1)
    {
      rtLogError("failed to create pipe. %s", rtStrError(ret).c_str());
    }
  }

  rtError start()
  {
    m_thread.reset(new std::thread(&rtRemoteStreamSelector::pollFds, this));
    return RT_OK;
  }

  rtError registerStream(std::shared_ptr<rtRemoteStream> const& s)
  {
    m_streams.push_back(s);
    return RT_OK;
  }

  rtError shutdown()
  {
    if (m_thread)
    {
      char buff[] = { "shudown" };
      write(m_shutdown_pipe[1], buff, sizeof(buff));

      m_thread->join();
      m_thread.reset();
    }

    ::close(m_shutdown_pipe[0]);
    ::close(m_shutdown_pipe[1]);

    return RT_OK;
  }

private:
  rtError pollFds()
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
      FD_ZERO(&err_fds);

      for (auto const& s : m_streams)
      {
        rtPushFd(&read_fds, s->m_fd, &maxFd);
        rtPushFd(&err_fds, s->m_fd, &maxFd);
      }
      rtPushFd(&read_fds, m_shutdown_pipe[0], &maxFd);

      timeval timeout;
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, &timeout);
      if (ret == -1)
      {
        rtLogWarn("select failed: %s", rtStrError(errno).c_str());
        continue;
      }

      if (FD_ISSET(m_shutdown_pipe[0], &read_fds))
      {
        rtLogInfo("got shutdown signal");
        return RT_OK;
      }

      time_t now = time(0);

      std::unique_lock<std::mutex> lock(m_mutex);
      for (int i = 0, n = static_cast<int>(m_streams.size()); i < n; ++i)
      {
        rtError e = RT_OK;
        std::shared_ptr<rtRemoteStream> const& s = m_streams[i];
        if (FD_ISSET(s->m_fd, &read_fds))
        {
          e = s->onIncomingMessage(buff, now);
          if (e != RT_OK)
            m_streams[i].reset();
        }
        else if (now - s->m_last_message_time > 2)
        {
          e = s->onInactivity(now);
          if (e != RT_OK)
            m_streams[i].reset();
        }
      }

      // remove all dead streams
      auto end = std::remove_if(m_streams.begin(), m_streams.end(),
          [](std::shared_ptr<rtRemoteStream> const& s) { return s == nullptr; });
      m_streams.erase(end, m_streams.end());
    }

    return RT_OK;
  }

private:
  std::vector< std::shared_ptr<rtRemoteStream> >  m_streams;
  std::shared_ptr< std::thread >                  m_thread;
  std::mutex                                      m_mutex;
  int                                             m_shutdown_pipe[2];
};

static std::shared_ptr<rtRemoteStreamSelector> gStreamSelector;

rtError
rtRemoteShutdownStreamSelector()
{
  if (gStreamSelector)
  {
    rtError e = gStreamSelector->shutdown();
    if (e != RT_OK)
      rtLogWarn("failed to shutdown StreamSelector. %s", rtStrError(e));
    gStreamSelector.reset();
  }
  return RT_OK;
}

rtRemoteStream::rtRemoteStream(int fd, sockaddr_storage const& local_endpoint, sockaddr_storage const& remote_endpoint)
  : m_fd(fd)
  , m_last_message_time(0)
  , m_message_handler(nullptr)
  , m_running(false)
{
  memcpy(&m_remote_endpoint, &remote_endpoint, sizeof(m_remote_endpoint));
  memcpy(&m_local_endpoint, &local_endpoint, sizeof(m_local_endpoint));
}

rtRemoteStream::~rtRemoteStream()
{
  this->close();
}

void
rtRemoteStream::runMessageDispatch()
{
  WorkItem item;

  while (true)
  {
    item.Doc = nullptr;
    {
      std::unique_lock<std::mutex> qlock(m_work_mutex);
      m_work_cond.wait(qlock, [this] { return !this->m_running || !m_work_queue.empty(); });

      if (!m_running)
        return;

      if (!m_work_queue.empty())
      {
        item = m_work_queue.front();
        m_work_queue.pop_front();
      }
    }

    if (item.Doc != nullptr)
    {
      if (m_message_handler)
      {
        rtError err = m_message_handler(item.Doc);
        if (err != RT_OK)
          rtLogWarn("error running message dispatcher: %s", rtStrError(err));
      }
    }
  }
}

rtError
rtRemoteStream::open()
{
  m_running = true;

  int const kNumDispatcherThreads = 1;
  for (int i = 0; i < kNumDispatcherThreads; ++i)
  {
    m_dispatch_threads.push_back(new std::thread(&rtRemoteStream::runMessageDispatch, this));
  }

  if (!gStreamSelector)
  {
    gStreamSelector.reset(new rtRemoteStreamSelector());
    gStreamSelector->start();
  }
  gStreamSelector->registerStream(shared_from_this());
  return RT_OK;
}

rtError
rtRemoteStream::close()
{
  {
    std::unique_lock<std::mutex> lock(m_work_mutex);
    m_running = false;
    m_work_cond.notify_all();
  }

  for (auto thread : m_dispatch_threads)
  {
    thread->join();
    delete thread;
  }

  m_dispatch_threads.clear();

  if (m_fd != kInvalidSocket)
  {
    int ret = 0;
    
    ret = ::shutdown(m_fd, SHUT_RDWR);
    if (ret == -1)
      rtLogDebug("shutdown failed on fd %d: %s", m_fd, rtStrError(errno).c_str());

    rtCloseSocket(m_fd);
  }

  return RT_OK;
}

rtError
rtRemoteStream::connect()
{
  assert(m_fd == kInvalidSocket);
  return connectTo(m_remote_endpoint);
}

rtError
rtRemoteStream::connectTo(sockaddr_storage const& endpoint)
{
  m_fd = socket(endpoint.ss_family, SOCK_STREAM, 0);
  if (m_fd < 0)
  {
    rtLogError("failed to create socket. %s", rtStrError(errno).c_str());
    return RT_FAIL;
  }
  uint32_t one = 1;
  if (-1 == setsockopt(m_fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one)))
    rtLogError("setting TCP_NODELAY failed");

  fcntl(m_fd, F_SETFD, fcntl(m_fd, F_GETFD) | FD_CLOEXEC);

  socklen_t len;
  rtSocketGetLength(endpoint, &len);

  int ret = ::connect(m_fd, reinterpret_cast<sockaddr const *>(&endpoint), len);
  if (ret < 0)
  {
    rtLogError("failed to connect to remote rpc endpoint. %s", rtStrError(errno).c_str());
    rtCloseSocket(m_fd);
    return RT_FAIL;
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
rtRemoteStream::send(rtRemoteMessage const& m)
{
  m_last_message_time = time(0);
  return m.send(m_fd, NULL);
}

rtError
rtRemoteStream::setMessageCallback(MessageHandler handler)
{
  m_message_handler = handler;
  return RT_OK;
}

rtError
rtRemoteStream::setInactivityCallback(rtRemoteInactivityHandler handler)
{
  m_inactivity_handler = handler;
  return RT_OK;
}

rtError
rtRemoteStream::onInactivity(time_t now)
{
  if (m_inactivity_handler)
  {
    m_inactivity_handler(m_last_message_time, now);
    m_last_message_time = now;
  }
  return RT_OK;
}

rtError
rtRemoteStream::onIncomingMessage(rtSocketBuffer& buff, time_t now)
{
  m_last_message_time = now;

  rtJsonDocPtr doc;

  rtError err = rtReadMessage(m_fd, buff, doc);
  if (err != RT_OK)
  {
    rtLogDebug("failed to read message from fd:%d. %s", m_fd, rtStrError(err));
    return err;
  }

  int const key = rtMessage_GetCorrelationKey(*doc);

  // is someone waiting for a response
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_requests.find(key) != m_requests.end())
    {
      m_requests[key] = doc;
      lock.unlock();
      m_cond.notify_all();
    }
  }

  // enqueue to dispatch on a worker thread
  {
    WorkItem item;
    item.Doc = doc;

    std::unique_lock<std::mutex> lock(m_work_mutex);
    m_work_queue.push_back(item);
    m_work_cond.notify_one();
  }

  return RT_OK;
}

rtError
rtRemoteStream::sendRequest(rtRemoteRequest const& req, MessageHandler handler, uint32_t timeout)
{
  rtCorrelationKey key = req.getCorrelationKey();
  assert(key != 0);
  assert(m_fd != kInvalidSocket);

  if (handler)
  {
    // prime outstanding request table. This indicates to the callbacks that someone
    // is waiting for a response
    std::unique_lock<std::mutex> lock(m_mutex);
    assert(m_requests.find(key) == m_requests.end());
    m_requests[key] = rtJsonDocPtr();
  }

  m_last_message_time = time(0);
  rtError e = req.send(m_fd, NULL);
  if (e != RT_OK)
  {
    rtLogWarn("failed to send request: %d", e);
    return e;
  }

  if (handler)
  {
    rtJsonDocPtr doc = waitForResponse(key, timeout);
    if (doc)
      e = handler(doc);
    else
      e = RT_TIMEOUT;
  }

  return e;
}

rtJsonDocPtr
rtRemoteStream::waitForResponse(int key, uint32_t timeout)
{
  rtJsonDocPtr res;

  auto delay = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);

  std::unique_lock<std::mutex> lock(m_mutex);
  m_cond.wait_until(lock, delay, [this, key, &res] 
    {
      auto itr = this->m_requests.find(key);
      if (itr != this->m_requests.end())
      {
        res = itr->second;
        if (res != nullptr)
          this->m_requests.erase(itr);
      }
      return res != nullptr;
    });
  lock.unlock();
  return res;
}
