#include "rtRpcStream.h"
#include "rtRpcMessage.h"

#include <algorithm>
#include <memory>
#include <thread>
#include <vector>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <rtLog.h>


class rtRpcStreamSelector
{
public:
  rtError start()
  {
    m_thread.reset(new std::thread(&rtRpcStreamSelector::pollFds, this));
    return RT_OK;
  }

  rtError registerStream(std::shared_ptr<rtRpcStream> const& s)
  {
    m_streams.push_back(s);
    return RT_OK;
  }

private:
  rtError pollFds()
  {
    rt_sockbuf_t buff;
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

      timeval timeout;
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, &timeout);
      if (ret == -1)
      {
	rtLogWarn("select failed: %s", rtStrError(errno).c_str());
	continue;
      }

      time_t now = time(0);

      std::unique_lock<std::mutex> lock(m_mutex);
      for (int i = 0, n = static_cast<int>(m_streams.size()); i < n; ++i)
      {
	rtError e = RT_OK;
	std::shared_ptr<rtRpcStream> const& s = m_streams[i];
	if (FD_ISSET(s->m_fd, &read_fds))
	{
	  e = s->onIncomingMessage(buff, now);
	  if (e != RT_OK)
	    m_streams[i].reset();
	}
	else if (now - s->m_last_message_time > 10)
	{
	  e = s->onInactivity(now);
	  if (e != RT_OK)
	    m_streams[i].reset();
	}
      }

      // remove all dead streams
      auto end = std::remove_if(m_streams.begin(), m_streams.end(),
	  [](std::shared_ptr<rtRpcStream> const& s) { return s == nullptr; });
      m_streams.erase(end, m_streams.end());
    }

    return RT_OK;
  }

private:
  std::vector< std::shared_ptr<rtRpcStream> > 	m_streams;
  std::shared_ptr< std::thread > 		m_thread;
  std::mutex					m_mutex;
};

static std::shared_ptr<rtRpcStreamSelector> gStreamSelector;


rtRpcStream::rtRpcStream(int fd)
  : m_fd(fd)
  , m_last_message_time(0)
  , m_message_handler(nullptr)
{
}

rtRpcStream::~rtRpcStream()
{
  this->close();
}

rtError
rtRpcStream::open()
{
  if (!gStreamSelector)
  {
    gStreamSelector.reset(new rtRpcStreamSelector());
    gStreamSelector->start();
  }
  gStreamSelector->registerStream(shared_from_this());
  return RT_OK;
}

rtError
rtRpcStream::close()
{
  if (m_fd > 0)
  {
    int ret = 0;
    
    ret = ::shutdown(m_fd, SHUT_RDWR);
    if (ret == -1)
      rtLogWarn("shutdown failed on fd %d: %s", m_fd, rtStrError(errno).c_str());

    ret = ::close(m_fd);
    if (ret == -1)
      rtLogWarn("close failed on fd %d: %s", m_fd, rtStrError(errno).c_str());
  }
  return RT_OK;
}

rtError
rtRpcStream::connectTo(sockaddr_storage const& endpoint)
{
  m_fd = socket(endpoint.ss_family, SOCK_STREAM, 0);
  if (m_fd < 0)
  {
    rtLogError("failed to create socket. %s", rtStrError(errno).c_str());
    return RT_FAIL;
  }
  fcntl(m_fd, F_SETFD, fcntl(m_fd, F_GETFD) | FD_CLOEXEC);

  socklen_t len;
  rtSocketGetLength(endpoint, &len);

  int ret = connect(m_fd, reinterpret_cast<sockaddr const *>(&endpoint), len);
  if (ret < 0)
  {
    rtLogError("failed to connect to remote rpc endpoint. %s", rtStrError(errno).c_str());
    return RT_FAIL;
  }

  rtLogInfo("new tcp connection to: %s", rtSocketToString(endpoint).c_str());
  return RT_OK;
}

rtError
rtRpcStream::send(rtRpcMessage const& m)
{
  m_last_message_time = time(0);
  return m.send(m_fd, NULL);
}

rtError
rtRpcStream::setMessageCallback(message_handler handler)
{
  m_message_handler = handler;
  return RT_OK;
}

rtError
rtRpcStream::setInactivityCallback(inactivity_handler handler)
{
  m_inactivity_handler = handler;
  return RT_OK;
}

rtError
rtRpcStream::onInactivity(time_t now)
{
  if (m_inactivity_handler)
  {
    m_inactivity_handler(m_last_message_time, now);
    m_last_message_time = now;
  }
  return RT_OK;
}

rtError
rtRpcStream::onIncomingMessage(rt_sockbuf_t& buff, time_t now)
{
  m_last_message_time = now;

  rtJsonDocPtr_t doc;
  rtError err = rtReadMessage(m_fd, buff, doc);
  if (err != RT_OK)
  {
    rtLogWarn("failed to read message from fd:%d. %d", m_fd, err);
    return err;
  }

  int const key = rtMessage_GetCorrelationKey(*doc);

  // is someone waiting for a response?
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    auto itr = m_requests.find(key);
    if (itr != m_requests.end())
    {
      m_requests[key] = doc;
      lock.unlock();
      m_cond.notify_all();
      return RT_OK;
    }
  }

  if (m_message_handler)
  {
    m_message_handler(doc);
  }

  return RT_OK;
}

rtError
rtRpcStream::sendRequest(rtRpcRequest const& req, message_handler handler, uint32_t timeout)
{
  rtCorrelationKey_t key = req.getCorrelationKey();
  assert(key != 0);

  if (handler)
  {
    // prime outstanding request table. This indicates to the callbacks that someone
    // is waiting for a response
    std::unique_lock<std::mutex> lock(m_mutex);
    assert(m_requests.find(key) == m_requests.end());
    m_requests[key] = rtJsonDocPtr_t();
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
    rtJsonDocPtr_t doc = waitForResponse(key, timeout);
    if (doc)
    {
      e = handler(doc);
    }
  }

  return e;
}

rtJsonDocPtr_t
rtRpcStream::waitForResponse(int key, uint32_t timeout)
{
  rtJsonDocPtr_t res;

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
