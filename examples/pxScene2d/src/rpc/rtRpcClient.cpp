#include "rtRpcClient.h"
#include "rtRpcClient.h"
#include "rtSocketUtils.h"
#include "rtRpcMessage.h"

#include "rapidjson/rapidjson.h"

#include <rtLog.h>

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
rtRpcClient::startSession(std::string const& object_id)
{
  rtError err = RT_OK;

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember(kFieldNameMessageType, kMessageTypeOpenSessionRequest, doc.GetAllocator());
  doc.AddMember(kFieldNameObjectId, object_id, doc.GetAllocator());

  key_type key = m_next_key++;
  doc.AddMember(kFieldNameCorrelationKey, key, doc.GetAllocator());

  err = rtSendDocument(doc, m_fd, NULL);
  if (err != RT_OK)
    return err;

  return waitForResponse(key) != nullptr
    ? RT_OK
    : RT_FAIL;
}

rtError
rtRpcClient::sendKeepAlive()
{
  key_type key = m_next_key++;

  rapidjson::Document req;
  req.SetObject();
  req.AddMember(kFieldNameMessageType, kMessageTypeKeepAliveRequest, req.GetAllocator());
  req.AddMember(kFieldNameCorrelationKey, key, req.GetAllocator());

  rapidjson::Value ids(rapidjson::kArrayType);
  for (auto const& id : m_object_list)
    ids.PushBack(rapidjson::Value().SetString(id.c_str(), id.size()), req.GetAllocator());
  req.AddMember(kFieldNameKeepAliveIds, ids, req.GetAllocator());

  return rtSendDocument(req, m_fd, NULL);
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
    timeout.tv_sec = 1;
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
rtRpcClient::get(std::string const& id, char const* name, rtValue* value)
{
  rapidjson::Document req;
  req.SetObject();
  req.AddMember(kFieldNameMessageType, kMessageTypeGetByNameRequest, req.GetAllocator());
  req.AddMember(kFieldNamePropertyName, std::string(name), req.GetAllocator());
  return sendGet(id, req, value);
}

rtError
rtRpcClient::get(std::string const& id, uint32_t index, rtValue* value)
{
  rapidjson::Document req;
  req.SetObject();
  req.AddMember(kFieldNameMessageType, kMessageTypeGetByIndexRequest, req.GetAllocator());
  req.AddMember(kFieldNamePropertyIndex, index, req.GetAllocator());
  return sendGet(id, req, value);
}

rtError
rtRpcClient::sendGet(std::string const& id, rapidjson::Document& req, rtValue* value)
{
  key_type key = m_next_key++;
  req.AddMember(kFieldNameObjectId, id, req.GetAllocator());
  req.AddMember(kFieldNameCorrelationKey, key, req.GetAllocator());

  rtError err = rtSendDocument(req, m_fd, NULL);
  if (err != RT_OK)
    return err;

  rtJsonDocPtr_t res = waitForResponse(key);
  if (!res)
    return RT_FAIL;

  auto itr = res->FindMember(kFieldNameValue);
  if (itr == res->MemberEnd())
    return RT_FAIL;

  err = rtValueReader::read(*value, itr->value, shared_from_this());
  if (err != RT_OK)
    return err;

  return rtMessage_GetStatusCode(*res);
}
  
rtError
rtRpcClient::set(std::string const& id, char const* name, rtValue const* value)
{
  rapidjson::Document req;
  req.SetObject();
  req.AddMember(kFieldNameMessageType, kMessageTypeSetByNameRequest, req.GetAllocator());
  req.AddMember(kFieldNamePropertyName, std::string(name), req.GetAllocator());
  return sendSet(id, req, value);
}

rtError
rtRpcClient::set(std::string const& id, uint32_t index, rtValue const* value)
{
  rapidjson::Document req;
  req.SetObject();
  req.AddMember(kFieldNameMessageType, kMessageTypeSetByIndexRequest, req.GetAllocator());
  req.AddMember(kFieldNamePropertyIndex, index, req.GetAllocator());
  return sendSet(id, req, value);
}

rtError
rtRpcClient::sendSet(std::string const& id, rapidjson::Document& req, rtValue const* value)
{
  key_type key = m_next_key++;
  req.AddMember(kFieldNameObjectId, id, req.GetAllocator());
  req.AddMember(kFieldNameCorrelationKey, key, req.GetAllocator());

  rapidjson::Value val;
  rtError err = rtValueWriter::write(*value, val, req);
  if (err != RT_OK)
    return err;
  req.AddMember(kFieldNameValue, val, req.GetAllocator());

  err = rtSendDocument(req, m_fd, NULL);
  if (err != RT_OK)
    return err;

  rtJsonDocPtr_t res = waitForResponse(key);
  if (!res)
    return RT_FAIL;

  return rtMessage_GetStatusCode(*res);
}

rtError
rtRpcClient::send(std::string const& id, std::string const& name, int argc, rtValue const* argv, rtValue* result)
{
  rtError err = RT_OK;

  key_type key = m_next_key++;

  rapidjson::Document req;
  req.SetObject();
  req.AddMember(kFieldNameMessageType, kMessageTypeMethodCallRequest, req.GetAllocator());
  if (id.size() > 0)
    req.AddMember(kFieldNameObjectId, id, req.GetAllocator());
  req.AddMember(kFieldNameFunctionName, name, req.GetAllocator());
  req.AddMember(kFieldNameCorrelationKey, key, req.GetAllocator());

  rapidjson::Value args(rapidjson::kArrayType);
  for (int i = 0; i < argc; ++i)
  {
    rapidjson::Value arg;
    rtError err = rtValueWriter::write(argv[i], arg, req);
    if (err != RT_OK)
      return err;
    args.PushBack(arg, req.GetAllocator());
  }
  req.AddMember(kFieldNameFunctionArgs, args, req.GetAllocator());

  err = rtSendDocument(req, m_fd, NULL);
  if (err != RT_OK)
    return err;

  rtJsonDocPtr_t res = waitForResponse(key);
  if (!res)
    return RT_FAIL;
  
  auto itr = res->FindMember(kFieldNameFunctionReturn);
  if (itr == res->MemberEnd())
    return RT_FAIL;

  err = rtValueReader::read(*result, itr->value, shared_from_this());
  if (err != RT_OK)
    return err;

  return rtMessage_GetStatusCode(*res);
}
