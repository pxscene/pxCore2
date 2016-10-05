#include "rtRemoteClient.h"
#include "rtRemoteClient.h"
#include "rtRemoteSocketUtils.h"
#include "rtRemoteMessage.h"
#include "rtRemoteValueReader.h"
#include "rtRemoteValueWriter.h"
#include "rtRemoteConfig.h"
#include "rtRemoteEnvironment.h"

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
  addValue(rtRemoteMessagePtr& doc, rtRemoteEnvironment* env, rtValue const& value)
  {
    rapidjson::Value jsonValue;
    rtError e = rtRemoteValueWriter::write(env, value, jsonValue, *doc);

    // TODO: better error handling
    if (e == RT_OK)
      doc->AddMember(kFieldNameValue, jsonValue, doc->GetAllocator());
  }

  void
  addArgument(rtRemoteMessagePtr& doc, rtRemoteEnvironment* env, rtValue const& value)
  {
    rapidjson::Value jsonValue;
    rtError e = rtRemoteValueWriter::write(env, value, jsonValue, *doc);

    // TODO: better error handling
    if (e == RT_OK)
    {
      auto itr = doc->FindMember(kFieldNameFunctionArgs);
      if (itr == doc->MemberEnd())
      {
        rapidjson::Value args(rapidjson::kArrayType);
        args.PushBack(jsonValue, doc->GetAllocator());
        doc->AddMember(kFieldNameFunctionArgs, args, doc->GetAllocator());
      }
      else
      {
        itr->value.PushBack(jsonValue, doc->GetAllocator());
      }
    }
  }
}

rtRemoteClient::rtRemoteClient(rtRemoteEnvironment* env, int fd,
  sockaddr_storage const& local_endpoint, sockaddr_storage const& remoteEndpoint)
  : m_stream(new rtRemoteStream(env, fd, local_endpoint, remoteEndpoint))
  , m_env(env)
{
  m_stream->setStateChangedHandler(&rtRemoteClient::onStreamStateChanged_Dispatcher, this);
  m_stream->setMessageHandler(&rtRemoteClient::onIncomingMessage_Dispatcher, this);
}

rtRemoteClient::rtRemoteClient(rtRemoteEnvironment* env, sockaddr_storage const& remoteEndpoint)
  : m_stream(new rtRemoteStream(env, -1, sockaddr_storage(), remoteEndpoint))
  , m_env(env)
{
  m_stream->setStateChangedHandler(&rtRemoteClient::onStreamStateChanged_Dispatcher, this);
  m_stream->setMessageHandler(&rtRemoteClient::onIncomingMessage_Dispatcher, this);
}

rtRemoteClient::~rtRemoteClient()
{
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_stream)
    {
      m_stream->close();
      m_stream.reset();
    }
  }
}

rtError
rtRemoteClient::send(rtRemoteMessagePtr const& msg)
{
  std::shared_ptr<rtRemoteStream> s = getStream();
  if (!s)
    return RT_ERROR_STREAM_CLOSED;
  return s->send(msg);
}

rtError
rtRemoteClient::onIncomingMessage(rtRemoteMessagePtr const& doc)
{
  std::shared_ptr<rtRemoteStream> s = getStream();
  if (!s)
    return RT_ERROR_STREAM_CLOSED;

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

    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_stream)
    {
      m_stream->close();
      m_stream.reset();
    }
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

  std::shared_ptr<rtRemoteStream> s = getStream();
  if (!s)
    return RT_ERROR_STREAM_CLOSED;

  err = s->open();
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
  std::shared_ptr<rtRemoteStream> s = getStream();
  if (!s)
    return RT_ERROR_STREAM_CLOSED;
  if (!s->isConnected())
    e = s->connect();
  return e;
}

rtError
rtRemoteClient::startSession(std::string const& objectId, uint32_t timeout)
{
  rtRemoteCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtRemoteMessagePtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeOpenSessionRequest, req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k.toString(), req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectId, req->GetAllocator());

  std::shared_ptr<rtRemoteStream> s = getStream();
  if (!s)
    return RT_ERROR_STREAM_CLOSED;

  rtRemoteAsyncHandle handle = s->sendWithWait(req, k);
  rtError e = handle.wait(timeout);
  if (e != RT_OK)
    rtLogDebug("e: %s", rtStrError(e));

  if (e == RT_OK)
  {
    rtRemoteMessagePtr res = handle.response();
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

  rtRemoteCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtRemoteMessagePtr msg(new rapidjson::Document());
  msg->SetObject();
  msg->AddMember(kFieldNameMessageType, kMessageTypeKeepAliveRequest, msg->GetAllocator());
  msg->AddMember(kFieldNameCorrelationKey,k.toString(), msg->GetAllocator());

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

  std::shared_ptr<rtRemoteStream> s = getStream();
  if (!s)
    return RT_ERROR_STREAM_CLOSED;
  return s->send(msg);
}

rtError
rtRemoteClient::sendSet(std::string const& objectId, char const* propertyName, rtValue const& value)
{
  rtRemoteCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtRemoteMessagePtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeSetByNameRequest, req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectId, req->GetAllocator());
  req->AddMember(kFieldNamePropertyName, std::string(propertyName), req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k.toString(), req->GetAllocator());
  addValue(req, m_env, value);

  return sendSet(req, k);
}

rtError
rtRemoteClient::sendSet(std::string const& objectId, uint32_t propertyIdx, rtValue const& value)
{
  rtRemoteCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtRemoteMessagePtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeSetByIndexRequest, req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectId, req->GetAllocator());
  req->AddMember(kFieldNamePropertyIndex, propertyIdx, req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k.toString(), req->GetAllocator());
  addValue(req, m_env, value);

  return sendSet(req, k);
}

rtError
rtRemoteClient::sendSet(rtRemoteMessagePtr const& req, rtRemoteCorrelationKey k)
{
  std::shared_ptr<rtRemoteStream> s = getStream();
  if (!s)
    return RT_ERROR_STREAM_CLOSED;

  rtRemoteAsyncHandle handle = s->sendWithWait(req, k);

  rtError e = handle.wait(0);
  if (e == RT_OK)
  {
    rtRemoteMessagePtr res = handle.response();
    if (!res)
      return RT_ERROR_PROTOCOL_ERROR;

    e = rtMessage_GetStatusCode(*res);
  }
  return e;
}

rtError
rtRemoteClient::sendGet(std::string const& objectId, char const* propertyName, rtValue& result)
{
  rtRemoteCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtRemoteMessagePtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeGetByNameRequest, req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectId, req->GetAllocator());
  req->AddMember(kFieldNamePropertyName, std::string(propertyName), req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k.toString(), req->GetAllocator());

  return sendGet(req, k, result);
}

rtError
rtRemoteClient::sendGet(std::string const& objectId, uint32_t propertyIdx, rtValue& result)
{
  rtRemoteCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtRemoteMessagePtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeGetByNameRequest, req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectId, req->GetAllocator());
  req->AddMember(kFieldNamePropertyIndex, propertyIdx, req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k.toString(), req->GetAllocator());

  return sendGet(req, k, result);
}

rtError
rtRemoteClient::sendGet(rtRemoteMessagePtr const& req, rtRemoteCorrelationKey k, rtValue& value)
{
  std::shared_ptr<rtRemoteStream> s = getStream();
  if (!s)
    return RT_ERROR_STREAM_CLOSED;

  rtRemoteAsyncHandle handle = s->sendWithWait(req, k);

  rtError e = handle.wait(0);
  if (e == RT_OK)
  {
    rtRemoteMessagePtr res = handle.response();
    if (!res)
      return RT_ERROR_PROTOCOL_ERROR;
    rtError statusCode = rtMessage_GetStatusCode(*res);
    if (statusCode != RT_OK)
    {
       return statusCode;
    }
    auto itr = res->FindMember(kFieldNameValue);
    if (itr == res->MemberEnd())
      return RT_ERROR_PROTOCOL_ERROR;

    e = rtRemoteValueReader::read(value, itr->value, shared_from_this());
    if (e == RT_OK)
      e = rtMessage_GetStatusCode(*res);
  }
  return e;
}

rtError
rtRemoteClient::sendCall(std::string const& objectId, std::string const& methodName,
  int argc, rtValue const* argv, rtValue& result)
{
  rtRemoteCorrelationKey k = rtMessage_GetNextCorrelationKey();

  rtRemoteMessagePtr req(new rapidjson::Document());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeMethodCallRequest, req->GetAllocator());
  req->AddMember(kFieldNameObjectId, objectId, req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, k.toString(), req->GetAllocator());
  req->AddMember(kFieldNameFunctionName, methodName, req->GetAllocator());
  
  for (int i = 0; i < argc; ++i)
    addArgument(req, m_env, argv[i]);
  
  return sendCall(req, k, result);
}

rtError
rtRemoteClient::sendCall(rtRemoteMessagePtr const& req, rtRemoteCorrelationKey k, rtValue& result)
{
  std::shared_ptr<rtRemoteStream> s = getStream();
  if (!s)
    return RT_ERROR_STREAM_CLOSED;

  rtRemoteAsyncHandle handle = s->sendWithWait(req, k);

  rtError e = handle.wait(0);
  if (e == RT_OK)
  {
    rtRemoteMessagePtr res = handle.response();
    if (!res)
      return RT_ERROR_PROTOCOL_ERROR;

    auto itr = res->FindMember(kFieldNameFunctionReturn);
    if (itr == res->MemberEnd())
      return RT_ERROR_PROTOCOL_ERROR;

    e = rtRemoteValueReader::read(result, itr->value, shared_from_this());
    if (e == RT_OK)
      e = rtMessage_GetStatusCode(*res);
  }
  return e;
}

sockaddr_storage
rtRemoteClient::getRemoteEndpoint() const
{
  sockaddr_storage saddr;
  memset(&saddr, 0, sizeof(sockaddr_storage));
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_stream)
      saddr = m_stream->getRemoteEndpoint();
  }
  return std::move(saddr);
}

sockaddr_storage
rtRemoteClient::getLocalEndpoint() const
{
  sockaddr_storage saddr;
  memset(&saddr, 0, sizeof(sockaddr_storage));
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_stream)
      m_stream->getLocalEndpoint();
  }
  return std::move(saddr);
}
