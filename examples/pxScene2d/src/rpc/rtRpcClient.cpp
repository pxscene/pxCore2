#include "rtRpcClient.h"
#include "rtRpcClient.h"
#include "rtSocketUtils.h"
#include "rtRpcMessage.h"

#include "rapidjson/rapidjson.h"

#include <rtLog.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <rapidjson/document.h>

rtRpcClient::rtRpcClient(sockaddr_storage const& ss)
  : m_fd(-1)
  , m_next_key(1)
{
  m_remote_endpoint = ss;
}

rtRpcClient::~rtRpcClient()
{
  if (m_fd > 0)
  {
    shutdown(m_fd, SHUT_RDWR);
    close(m_fd);
  }
}

rtError
rtRpcClient::start()
{
  rtError err = connectRpcEndpoint();
  if (err != RT_OK)
  {
    rtLogWarn("failed to connect to rpc endpoint");
    return err;
  }

  m_thread.reset(new std::thread(&rtRpcClient::runListener, this));
  return RT_OK;
}

rtError
rtRpcClient::connectRpcEndpoint()
{
  m_fd = socket(m_remote_endpoint.ss_family, SOCK_STREAM, 0);
  if (m_fd < 0)
  {
    rtLogError("failed to create socket. %s", strerror(errno));
    return RT_FAIL;
  }
  fcntl(m_fd, F_SETFD, fcntl(m_fd, F_GETFD) | FD_CLOEXEC);

  socklen_t len;
  rtSocketGetLength(m_remote_endpoint, &len);

  int ret = connect(m_fd, reinterpret_cast<sockaddr *>(&m_remote_endpoint), len);
  if (ret < 0)
  {
    rtLogError("failed to connect to remote rpc endpoint. %s", strerror(errno));
    return RT_FAIL;
  }

  rtLogInfo("new tcp connection to: %s", rtSocketToString(m_remote_endpoint).c_str());
  return RT_OK;
}

rtError
rtRpcClient::startSession(std::string const& objectName)
{
  rtError e = RT_FAIL;

  rtRpcRequestOpenSession req(objectName);
  e = req.send(m_fd, NULL);
  if (e != RT_OK)
    return e;

  return waitForResponse(req.getCorrelationKey()) != nullptr
    ? RT_OK
    : RT_FAIL;
}

rtError
rtRpcClient::sendKeepAlive()
{
  rtRpcRequestKeepAlive req;
  for (auto const& name : m_object_list)
    req.addObjectName(name);
  return req.send(m_fd, NULL);
}

rtError
rtRpcClient::runListener()
{
  rt_sockbuf_t buff;
  buff.reserve(1024 * 4);
  buff.resize(1024 * 4);

  while (true)
  {
    int maxFd = 0;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    rtPushFd(&read_fds, m_fd, &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_fd, &maxFd);

    // timeout is for keep-alive
    timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, &timeout);
    if (ret == 0)
    {
      rtError err = sendKeepAlive();
      if (err != RT_OK)
        rtLogWarn("error sending keep-alive:%d", err);
      continue;
    }

    if (ret == -1)
    {
      int err = errno;
      rtLogWarn("select failed. %s", strerror(errno));
      if (err == EBADF)
        return RT_FAIL;
      else
        continue;
    }

    if (FD_ISSET(m_fd, &read_fds))
    {
      rtError err = readn(m_fd, buff);
      if (err != RT_OK)
      {
        rtLogWarn("failed to read from fd: %d", m_fd);
        return err;
      }
    }
  }

  return RT_OK;
}

rtError
rtRpcClient::readn(int fd, rt_sockbuf_t& buff)
{
  rtJsonDocPtr_t doc;
  rtError err = rtReadMessage(fd, buff, doc);
  if (err != RT_OK)
    return err;

  auto type = doc->FindMember(kFieldNameMessageType);
  if (type == doc->MemberEnd())
  {
    rtLogWarn("received JSON message with no type");
    return RT_FAIL;
  }

  std::string cmd = type->value.GetString();

  // TODO: this could be done with std::future/std::promise
  int key = rtMessage_GetCorrelationKey(*doc);

  // should be std::future/std::promise
  if (key != -1)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_requests[key] = doc;
    lock.unlock();
    m_cond.notify_all();
  }

  // https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn
  #define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

  auto itr = m_message_handlers.find(cmd);
  if (itr != m_message_handlers.end())
    err = CALL_MEMBER_FN(*this, itr->second)(doc); // , peer);

  return err;
}

template<class TResponse> TResponse
rtRpcClient::waitForResponse2(rtRpcRequest const& req, uint32_t timeout)
{
  rtJsonDocPtr_t res;
  int key = req.getCorrelationKey();

  auto delay = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);

  std::unique_lock<std::mutex> lock(m_mutex);
  m_cond.wait_until(lock, delay, [this, key, &res]
    {
      auto itr = this->m_requests.find(key);
      if (itr != this->m_requests.end())
      {
        res = itr->second;
        this->m_requests.erase(itr);
      }
      return res != nullptr;
    });
  lock.unlock();

  if (!res)
    return TResponse(kMessageTypeInvalidResponse);

  auto itr = res->FindMember(kFieldNameMessageType);
  if (itr == res->MemberEnd())
  {
    rtLogError("failed to find %s in message", kFieldNameMessageType);
    return TResponse(kMessageTypeInvalidResponse);
  }

  return TResponse(itr->value.GetString());
}

rtJsonDocPtr_t
rtRpcClient::waitForResponse(int key, uint32_t timeout)
{
  rtJsonDocPtr_t response;

  // TODO: std::future/std::promise
  auto delay = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);

  std::unique_lock<std::mutex> lock(m_mutex);
  m_cond.wait_until(lock, delay, [this, key, &response] 
    {
      auto itr = this->m_requests.find(key);
      if (itr != this->m_requests.end())
      {
        response = itr->second;
        this->m_requests.erase(itr);
      }
      return response != nullptr;
    });
  lock.unlock();

  return response;
}

rtError
rtRpcClient::get(std::string const& objectName, char const* propertyName, rtValue& value)
{
  return sendGet(rtRpcGetRequest(objectName, propertyName), value);
}

rtError
rtRpcClient::get(std::string const& objectName, uint32_t index, rtValue& value)
{
  return sendGet(rtRpcGetRequest(objectName, index), value);
}

rtError
rtRpcClient::sendGet(rtRpcGetRequest const& req, rtValue& value)
{
  rtError e = RT_FAIL;

  e = req.send(m_fd, NULL);
  if (e != RT_OK)
    return e;

  #if 0 // we'd really like it to look like this
  rtRpcGetResponse res = waitForResponse2<rtRpcGetResponse>(req);
  if (!res.isValid())
    return RT_FAIL;

  e = res.getStatusCode();
  if (e != RT_OK)
    return e;

  value = res.getValue();
  return RT_OK;
  #else
  rtJsonDocPtr_t res = waitForResponse(req.getCorrelationKey());
  if (!res)
    return RT_FAIL;

  auto itr = res->FindMember(kFieldNameValue);
  if (itr == res->MemberEnd())
    return RT_FAIL;

  e = rtValueReader::read(value, itr->value, shared_from_this());
  if (e != RT_OK)
    return e;

  return rtMessage_GetStatusCode(*res);
  #endif
}
  
rtError
rtRpcClient::set(std::string const& objectName, char const* propertyName, rtValue const& value)
{
  rtRpcSetRequest req(objectName, propertyName);
  req.setValue(value);
  return sendSet(req);
}

rtError
rtRpcClient::set(std::string const& objectName, uint32_t propertyIndex, rtValue const& value)
{
  rtRpcSetRequest req(objectName, propertyIndex);
  req.setValue(value);
  return sendSet(req);
}

rtError
rtRpcClient::sendSet(rtRpcSetRequest const& req)
{
  rtError e = RT_FAIL;

  e = req.send(m_fd, NULL);
  if (e != RT_OK)
    return e;

  rtJsonDocPtr_t res = waitForResponse(req.getCorrelationKey());
  if (!res)
    return RT_FAIL;

  return rtMessage_GetStatusCode(*res);
}

rtError
rtRpcClient::send(std::string const& objectName, std::string const& methodName,
  int argc, rtValue const* argv, rtValue* result)
{
  rtError e = RT_OK;

  rtRpcMethodCallRequest req(objectName);
  req.setMethodName(methodName);
  for (int i = 0; i < argc; ++i)
    req.addMethodArgument(argv[i]);

  e = req.send(m_fd, NULL);
  if (e != RT_OK)
    return e;

  rtJsonDocPtr_t res = waitForResponse(req.getCorrelationKey());
  if (!res)
    return RT_FAIL;
  
  auto itr = res->FindMember(kFieldNameFunctionReturn);
  if (itr == res->MemberEnd())
    return RT_FAIL;

  e = rtValueReader::read(*result, itr->value, shared_from_this());
  if (e != RT_OK)
    return e;

  return rtMessage_GetStatusCode(*res);
}
