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

namespace
{
  void
  AddValue(rtJsonDocPtr& doc, rtRemoteEnvironment* env, rtValue const& value)
  {
    rapidjson::Value json_value;
    rtError e = rtValueWriter::write(env, value, json_value, *doc);

    // TODO: better error handling
    if (e == RT_OK)
      doc->AddMember(kFieldNameValue, json_value, doc->GetAllocator());
  }
}

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
  if (m_stream)
    m_stream->close();
}

rtError
rtRemoteClient::send(rtJsonDocPtr const& msg)
{
  if (!m_stream)
    return RT_ERROR_INVALID_OPERATION;
  return m_stream->send(msg);
}

rtError
rtRemoteClient::onIncomingMessage(rtJsonDocPtr const& doc)
{
  if (!m_stream)
    return RT_ERROR_INVALID_OPERATION;

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
  rtCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtJsonDocPtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeOpenSessionRequest, req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k, req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectName, req->GetAllocator());

  rtRemoteAsyncHandle handle = m_stream->sendWithWait(req);
  rtError e = handle.wait(timeout);
  if (e == RT_OK)
  {
    rtJsonDocPtr res = handle.response();
  }

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
rtRemoteClient::sendSet(std::string const& objectId, char const* propertyName, rtValue const& value)
{
  rtCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtJsonDocPtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeSetByNameRequest, req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectId, req->GetAllocator());
  req->AddMember(kFieldNamePropertyName, std::string(propertyName), req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k, req->GetAllocator());
  AddValue(req, m_env, value);

  return sendSet(req, k);
}

rtError
rtRemoteClient::sendSet(std::string const& objectId, uint32_t propertyIdx, rtValue const& value)
{
  rtCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtJsonDocPtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeSetByIndexRequest, req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectId, req->GetAllocator());
  req->AddMember(kFieldNamePropertyIndex, propertyIdx, req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k, req->GetAllocator());

  return sendSet(req, k);
}

rtError
rtRemoteClient::sendGet(std::string const& objectId, char const* propertyName, rtValue& result)
{
  rtCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtJsonDocPtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeGetByNameRequest, req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectId, req->GetAllocator());
  req->AddMember(kFieldNamePropertyName, std::string(propertyName), req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k, req->GetAllocator());

  return sendGet(req, k, result);
}

rtError
rtRemoteClient::sendGet(std::string const& objectId, uint32_t propertyIdx, rtValue& result)
{
  rtCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtJsonDocPtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeGetByNameRequest, req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectId, req->GetAllocator());
  req->AddMember(kFieldNamePropertyIndex, propertyIdx, req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k, req->GetAllocator());

  return sendGet(req, k, result);
}

rtError
rtRemoteClient::sendGet(rtJsonDocPtr const& req, rtCorrelationKey k, rtValue& value)
{
  rtRemoteAsyncHandle handle = m_stream->sendWithWait(req);

  rtError e = handle.wait(0);
  if (e == RT_OK)
  {
    rtJsonDocPtr res = handle.response();
    if (!res)
      return RT_ERROR_PROTOCOL_ERROR;

    auto itr = res->FindMember(kFieldNameValue);
    if (itr == res->MemberEnd())
      return RT_ERROR_PROTOCOL_ERROR;

    e = rtValueReader::read(value, itr->value, shared_from_this());
    if (e == RT_OK)
      e = rtMessage_GetStatusCode(*res);
  }
  return e;
}

rtError
rtRemoteClient::sendSet(rtJsonDocPtr const& req, rtCorrelationKey k)
{
  rtRemoteAsyncHandle handle = m_stream->sendWithWait(req);

  rtError e = handle.wait(0);
  if (e == RT_OK)
  {
    rtJsonDocPtr res = handle.response();
    if (!res)
      return RT_ERROR_PROTOCOL_ERROR;

    e = rtMessage_GetStatusCode(*res);
  }
  return e;
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
