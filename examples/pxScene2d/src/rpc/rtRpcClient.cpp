#include "rtRpcClient.h"
#include "rtRpcClient.h"
#include "rtSocketUtils.h"
#include "rtRpcMessage.h"
#include "rtValueReader.h"
#include "rtValueWriter.h"
#include "rtRpcConfig.h"

#include "rapidjson/rapidjson.h"

#include <rtLog.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <rapidjson/document.h>

rtRpcClient::rtRpcClient(int fd, sockaddr_storage const& local_endpoint, sockaddr_storage const& remote_endpoint)
  : m_stream(new rtRpcStream(fd, local_endpoint, remote_endpoint))
{
  m_stream->setMessageCallback(std::bind(&rtRpcClient::onIncomingMessage, this,
    std::placeholders::_1));
  m_stream->setInactivityCallback(std::bind(&rtRpcClient::onInactivity, this,
    std::placeholders::_1, std::placeholders::_2));
}

rtRpcClient::rtRpcClient(sockaddr_storage const& remote_endpoint)
  : m_stream(new rtRpcStream(-1, sockaddr_storage(), remote_endpoint))
{
  m_stream->setMessageCallback(std::bind(&rtRpcClient::onIncomingMessage, this,
    std::placeholders::_1));
  m_stream->setInactivityCallback(std::bind(&rtRpcClient::onInactivity, this,
    std::placeholders::_1, std::placeholders::_2));
}

rtRpcClient::~rtRpcClient()
{
  m_stream->close();
}

rtError
rtRpcClient::onIncomingMessage(rtJsonDocPtr const& msg)
{
  rtError e = RT_OK;

  auto type = msg->FindMember(kFieldNameMessageType);
  if (type == msg->MemberEnd())
  {
    rtLogWarn("received JSON message with no type");
    e = RT_FAIL;
  }

  std::shared_ptr<rtRpcClient> client = shared_from_this();
  if (e == RT_OK && m_message_handler)
  {
    e = m_message_handler(client, msg);
    if (e != RT_OK)
      rtLogWarn("error inoking message handler. %s", rtStrError(e));
  }

  return e;
}

rtError
rtRpcClient::onInactivity(time_t /*lastMessage*/, time_t /*now*/)
{
  sendKeepAlive();
  return RT_OK;
}

rtError
rtRpcClient::open()
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
rtRpcClient::connectRpcEndpoint()
{
  rtError e = RT_OK;
  if (!m_stream->isConnected())
  {
    e = m_stream->connect();
  }
  return e;
}

rtError
rtRpcClient::startSession(std::string const& objectName, uint32_t timeout)
{
  rtJsonDocPtr res;
  rtRpcRequestOpenSession req(objectName);

  if (timeout == 0)
    timeout = rtRpcSetting<uint32_t>("rt.rpc.default.request_timeout");

  rtError e = m_stream->sendRequest(req, [&res](rtJsonDocPtr const& doc)
  {
    res = doc;
    return RT_OK;
  }, timeout);

  if (e != RT_OK)
  {
    rtLogError("failed to send request to start session. %s", rtStrError(e));
    return e;
  }

  return res != nullptr ? RT_OK : RT_FAIL;
}

rtError
rtRpcClient::sendKeepAlive()
{
  if (m_object_list.empty())
    return RT_OK;
  rtRpcRequestKeepAlive req;
  for (auto const& name : m_object_list)
    req.addObjectName(name);
  return m_stream->send(req);
}

rtError
rtRpcClient::get(std::string const& objectName, char const* propertyName, rtValue& value,
  uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRpcSetting<uint32_t>("rt.rpc.default.request_timeout");

  return sendGet(rtRpcGetRequest(objectName, propertyName), value, timeout);
}

rtError
rtRpcClient::get(std::string const& objectName, uint32_t index, rtValue& value,
  uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRpcSetting<uint32_t>("rt.rpc.default.request_timeout");

  return sendGet(rtRpcGetRequest(objectName, index), value, timeout);
}

rtError
rtRpcClient::sendGet(rtRpcGetRequest const& req, rtValue& value, uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRpcSetting<uint32_t>("rt.rpc.default.request_timeout");

  rtJsonDocPtr res;
  rtError e = m_stream->sendRequest(req, [&res](rtJsonDocPtr const& doc)
  {
    res = doc;
    return RT_OK;
  }, timeout);

  if (e != RT_OK)
    return e;

  if (!res)
    return RT_FAIL;

  auto itr = res->FindMember(kFieldNameValue);
  if (itr == res->MemberEnd())
    return RT_FAIL;

  e = rtValueReader::read(value, itr->value, shared_from_this());
  if (e != RT_OK)
    return e;

  return rtMessage_GetStatusCode(*res);
}
  
rtError
rtRpcClient::set(std::string const& objectName, char const* propertyName, rtValue const& value,
  uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRpcSetting<uint32_t>("rt.rpc.default.request_timeout");

  rtRpcSetRequest req(objectName, propertyName);
  req.setValue(value);
  return sendSet(req, timeout);
}

rtError
rtRpcClient::set(std::string const& objectName, uint32_t propertyIndex, rtValue const& value,
  uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRpcSetting<uint32_t>("rt.rpc.default.request_timeout");

  rtRpcSetRequest req(objectName, propertyIndex);
  req.setValue(value);
  return sendSet(req, timeout);
}

rtError
rtRpcClient::sendSet(rtRpcSetRequest const& req, uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRpcSetting<uint32_t>("rt.rpc.default.request_timeout");

  rtJsonDocPtr res;
  rtError e = m_stream->sendRequest(req, [&res](rtJsonDocPtr const& doc) 
  {
    res = doc;
    return RT_OK;
  }, timeout);

  if (e != RT_OK)
    return e;

  if (!res)
    return RT_FAIL;

  return rtMessage_GetStatusCode(*res);
}

rtError
rtRpcClient::send(std::string const& objectName, std::string const& methodName,
  int argc, rtValue const* argv, rtValue* result, uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRpcSetting<uint32_t>("rt.rpc.default.request_timeout");

  rtRpcMethodCallRequest req(objectName);
  req.setMethodName(methodName);
  for (int i = 0; i < argc; ++i)
    req.addMethodArgument(argv[i]);

  rtJsonDocPtr res;
  rtError e = m_stream->sendRequest(req, [&res](rtJsonDocPtr const& doc)
  {
    res = doc;
    return RT_OK;
  }, timeout);

  if (e != RT_OK)
    return e;

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
