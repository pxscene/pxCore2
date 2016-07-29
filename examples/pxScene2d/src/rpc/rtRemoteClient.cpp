#include "rtRemoteClient.h"
#include "rtRemoteClient.h"
#include "rtSocketUtils.h"
#include "rtRemoteMessage.h"
#include "rtValueReader.h"
#include "rtValueWriter.h"
#include "rtRemoteConfig.h"
#include "rtRemote.h"

#include "rapidjson/rapidjson.h"

#include <rtLog.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <algorithm>
#include <rapidjson/document.h>

rtRemoteClient::rtRemoteClient(rtRemoteEnvironment* env, int fd,
  sockaddr_storage const& local_endpoint, sockaddr_storage const& remote_endpoint)
  : m_stream(new rtRemoteStream(env, fd, local_endpoint, remote_endpoint))
  , m_env(env)
{
  m_stream->setStateChangedHandler(&rtRemoteClient::onStreamStateChanged_Dispatcher, this);
  m_stream->setMessageHandler(&rtRemoteClient::onIncomingMessage_Dispatcher, this);
}

rtRemoteClient::rtRemoteClient(rtRemoteEnvironment* env, sockaddr_storage const& remote_endpoint)
  : m_stream(new rtRemoteStream(env, -1, sockaddr_storage(), remote_endpoint))
  , m_env(env)
{
  m_stream->setStateChangedHandler(&rtRemoteClient::onStreamStateChanged_Dispatcher, this);
  m_stream->setMessageHandler(&rtRemoteClient::onIncomingMessage_Dispatcher, this);
}

rtRemoteClient::~rtRemoteClient()
{
  m_stream->close();
}

rtError
rtRemoteClient::send(rtJsonDocPtr const& msg)
{
  return m_stream->send(msg);
}

rtError
rtRemoteClient::onIncomingMessage(rtJsonDocPtr const& doc)
{
  auto self = shared_from_this();
  m_env->enqueueWorkItem(self, doc);
  return RT_OK;
}

rtError
rtRemoteClient::onInactivity(time_t /*lastMessage*/, time_t /*now*/)
{
  sendKeepAlive();
  return RT_OK;
}

rtError
rtRemoteClient::onStreamStateChanged(std::shared_ptr<rtRemoteStream> const& /*stream*/,
    rtRemoteStream::State state)
{
  if (state == rtRemoteStream::State::Closed)
  {
    rtLogInfo("stream closed");
    if (m_state_changed_handler.Func)
    {
      auto self = shared_from_this();
      rtError e = m_state_changed_handler.Func(self, State::Shutdown, m_state_changed_handler.Arg);
      if (e != RT_OK)
        rtLogWarn("failed to invoke state changed handler. %s", rtStrError(e));
    }
    if (m_stream)
      m_stream.reset();
  }
  return RT_OK;
}

rtError
rtRemoteClient::setStateChangedHandler(StateChangedHandler handler, void* argp)
{
  m_state_changed_handler.Func = handler;
  m_state_changed_handler.Arg = argp;
  return RT_OK;
}

rtError
rtRemoteClient::open()
{
  rtError err = connectRpcEndpoint();
  if (err != RT_OK)
  {
    rtLogWarn("failed to connect to rpc endpoint: %d", err);
    return err;
  }
  err = m_stream->open();
  if (err != RT_OK)
  {
    rtLogError("failed to open stream for read/write: %d", err);
    return err;
  }
  return RT_OK;
}

rtError
rtRemoteClient::connectRpcEndpoint()
{
  rtError e = RT_OK;
  if (!m_stream->isConnected())
  {
    e = m_stream->connect();
  }
  return e;
}

rtError
rtRemoteClient::startSession(std::string const& objectName, uint32_t timeout)
{
  rtJsonDocPtr res = nullptr;
  rtError e = RT_FAIL;
  
  assert(false);

  // register callback with env

  // send message

  // wait for response

  // remove callback with env

  // TODO:
  // rtError e = sendSynchronousRequest(rtRemoteRequestOpenSession(objectName), res, timeout);

  if (e != RT_OK)
    return e;

  return e;
}

void rtRemoteClient::keepAlive(std::string const& s)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_object_list.push_back(s);
}

void rtRemoteClient::removeKeepAlive(std::string const& s)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  auto it = std::find(m_object_list.begin(), m_object_list.end(), s);
  if (it != m_object_list.end()) m_object_list.erase(it);
}

rtError
rtRemoteClient::sendKeepAlive()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  if (m_object_list.empty())
    return RT_OK;

  rtJsonDocPtr msg(new rapidjson::Document());
  msg->SetObject();
  msg->AddMember(kFieldNameMessageType, kMessageTypeKeepAliveRequest, msg->GetAllocator());
  msg->AddMember(kFieldNameCorrelationKey, rtMessage_GetNextCorrelationKey(), msg->GetAllocator());

  for (std::string const& name : m_object_list)
  {
    auto itr = msg->FindMember(kFieldNameKeepAliveIds);
    if (itr == msg->MemberEnd())
    {
      rapidjson::Value ids(rapidjson::kArrayType);
      ids.PushBack(rapidjson::Value().SetString(name.c_str(), name.size()), msg->GetAllocator());
      msg->AddMember(kFieldNameKeepAliveIds, ids, msg->GetAllocator());
    }
    else
    {
      itr->value.PushBack(rapidjson::Value().SetString(name.c_str(), name.size()), msg->GetAllocator());
    }
  }
  lock.unlock();

  return m_stream->send(msg);
}

rtError
rtRemoteClient::set(std::string const& objectName, char const* propertyName, rtValue const& value,
  uint32_t timeout)
{
  rtRemoteSetRequest req(objectName, propertyName);
  req.setValue(m_env, value);
  return sendSet(req, timeout);
}

rtError
rtRemoteClient::set(std::string const& objectName, uint32_t propertyIndex, rtValue const& value,
  uint32_t timeout)
{
  rtRemoteSetRequest req(objectName, propertyIndex);
  req.setValue(m_env, value);
  return sendSet(req, timeout);
}

rtError
rtRemoteClient::waitForResponse(rtCorrelationKey k, rtJsonDocPtr& res, int timeout)
{
  #if 0
  rtError e = RT_ERROR_TIMEOUT;
  res = nullptr;

  if (timeout == 0)
    timeout = m_env->Config->environment_request_timeout();

  rtLogInfo("waiting for k:%d", (int) k);

  std::chrono::system_clock::time_point delay =
    std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);

  while ((res == nullptr) && (delay > std::chrono::system_clock::now()))
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait_until(lock, delay, [this, k, &e, &res, timeout]
    {
      // did our response arrive?
      auto itr = this->m_responses.find(k);
      if ((itr != this->m_responses.end()) && (itr->second != nullptr))
      {
        res = itr->second;
        this->m_responses.erase(itr);
        e = RT_OK;
        return true;
      }

      if (!m_env->Config->server_use_dispatch_thread())
        return !this->m_env->isQueueEmpty();

      return false;
    });

    if (res != nullptr)
      return RT_OK;

    rtError e = rtRemoteRunOnce(m_env, 0);
    if (e != RT_OK)
      rtLogDebug("rtRemoteRunOnce:%s", rtStrError(e));
  }

  // do one last check for response. this can happen if we spen a lot of time
  // draining the m_env queue
  std::unique_lock<std::mutex> lock(m_mutex);
  auto itr = m_responses.find(k);
  if ((itr != m_responses.end()) && (itr->second != nullptr))
  {
    res = itr->second;
    m_responses.erase(itr);
    e = RT_OK;
  }
  else if (itr != m_responses.end())
  {
    m_responses.erase(itr);
  }

  if (e == RT_ERROR_TIMEOUT)
    assert(false);

  return e;
  #endif
  return RT_OK;
}

rtError
rtRemoteClient::responseHandler(rtJsonDocPtr const& msg)
{
  auto self = shared_from_this();
  m_env->enqueueWorkItem(self, msg);
  return RT_OK;
}

#if 0
  rtCorrelationKey const k = rtMessage_GetCorrelationKey(*doc);

  std::unique_lock<std::mutex> lock(m_mutex);
  // only make entry if call is outstanding
  auto itr = m_responses.find(k);
  if (itr != m_responses.end())
  {
    itr->second = doc;
  }
  else
  {
    rtLogWarn("got callback for correlation key: %d, but no callback exists",
      (int) rtMessage_GetCorrelationKey(*doc));
  }
  lock.unlock();
  m_cond.notify_all();
  return RT_OK;
}
#endif

rtError
rtRemoteClient::sendSynchronousRequest(rtJsonDocPtr const& req, rtJsonDocPtr& res, int timeout)
{
  res = nullptr;

  rtCorrelationKey const k = rtMessage_GetCorrelationKey(*req);

  rtError e = m_stream->send(req);
  if (e != RT_OK)
    return e;

  e = waitForResponse(k, res, timeout);
  if (e != RT_OK)
  {
    rtLogDebug("error waiting for response with key:%d. %s",
      static_cast<int>(k), rtStrError(e));
    return e;
  }

  if (!res)
  {
    rtLogWarn("got null response");
    return RT_FAIL;
  }

  return RT_OK;
}

rtError
rtRemoteClient::sendSet(rtRemoteSetRequest const& req, uint32_t timeout)
{
#if 0
  rtJsonDocPtr res = nullptr;
  rtError e = sendSynchronousRequest(req, res, timeout);

  if (e != RT_OK)
    return e;

  if (!res)
    return RT_FAIL;

  return rtMessage_GetStatusCode(*res);
#endif
  return RT_OK;
}

rtError
rtRemoteClient::send(std::string const& objectName, std::string const& methodName,
  int argc, rtValue const* argv, rtValue* result, uint32_t timeout)
{
  rtRemoteMethodCallRequest req(objectName);
  req.setMethodName(methodName);
  for (int i = 0; i < argc; ++i)
    req.addMethodArgument(m_env, argv[i]);

  rtJsonDocPtr res = nullptr;
  rtError e = RT_FAIL; // sendSynchronousRequest(req, res, timeout);
  assert(false);

  if (e != RT_OK)
    return e;

  if (!res)
    return RT_FAIL;
  
  auto itr = res->FindMember(kFieldNameFunctionReturn);
  if (itr == res->MemberEnd())
    return RT_FAIL;

  if (result)
  {
    auto self = shared_from_this();
    e = rtValueReader::read(*result, itr->value, self);
    if (e != RT_OK)
      return e;
  }

  return rtMessage_GetStatusCode(*res);
}
