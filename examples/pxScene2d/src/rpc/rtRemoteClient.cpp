#include "rtRemoteClient.h"
#include "rtRemoteClient.h"
#include "rtSocketUtils.h"
#include "rtRemoteMessage.h"
#include "rtValueReader.h"
#include "rtValueWriter.h"
#include "rtRemoteConfig.h"

#include "rapidjson/rapidjson.h"

#include <rtLog.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <rapidjson/document.h>

rtRemoteClient::rtRemoteClient(int fd, sockaddr_storage const& local_endpoint, sockaddr_storage const& remote_endpoint)
  : m_stream(new rtRemoteStream(fd, local_endpoint, remote_endpoint))
  , m_message_handler(nullptr)
{
  m_stream->setMessageCallback(std::bind(&rtRemoteClient::onIncomingMessage, this,
    std::placeholders::_1));
  m_stream->setInactivityCallback(std::bind(&rtRemoteClient::onInactivity, this,
    std::placeholders::_1, std::placeholders::_2));
}

rtRemoteClient::rtRemoteClient(sockaddr_storage const& remote_endpoint)
  : m_stream(new rtRemoteStream(-1, sockaddr_storage(), remote_endpoint))
  , m_message_handler(nullptr)
{
  m_stream->setMessageCallback(std::bind(&rtRemoteClient::onIncomingMessage, this,
    std::placeholders::_1));
  m_stream->setInactivityCallback(std::bind(&rtRemoteClient::onInactivity, this,
    std::placeholders::_1, std::placeholders::_2));
}

rtRemoteClient::~rtRemoteClient()
{
  m_stream->close();
}

rtError
rtRemoteClient::onIncomingMessage(rtJsonDocPtr const& msg)
{
  std::shared_ptr<rtRemoteClient> client = shared_from_this();

  rtError e = RT_OK;
  if (m_message_handler)
  {
    e = m_message_handler(client, msg);
    if (e != RT_OK)
      rtLogWarn("error inoking message handler. %s", rtStrError(e));
  }

  return e;
}

rtError
rtRemoteClient::onInactivity(time_t /*lastMessage*/, time_t /*now*/)
{
  sendKeepAlive();
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
  rtJsonDocPtr res;
  rtRemoteRequestOpenSession req(objectName);

  if (timeout == 0)
    timeout = rtRemoteSetting<uint32_t>("rt.rpc.default.request_timeout");

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
rtRemoteClient::sendKeepAlive()
{
  if (m_object_list.empty())
    return RT_OK;
  rtRemoteRequestKeepAlive req;
  for (auto const& name : m_object_list)
    req.addObjectName(name);
  return m_stream->send(req);
}

rtError
rtRemoteClient::get(std::string const& objectName, char const* propertyName, rtValue& value,
  uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRemoteSetting<uint32_t>("rt.rpc.default.request_timeout");

  return sendGet(rtRemoteGetRequest(objectName, propertyName), value, timeout);
}

rtError
rtRemoteClient::get(std::string const& objectName, uint32_t index, rtValue& value,
  uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRemoteSetting<uint32_t>("rt.rpc.default.request_timeout");

  return sendGet(rtRemoteGetRequest(objectName, index), value, timeout);
}

rtError
rtRemoteClient::sendGet(rtRemoteGetRequest const& req, rtValue& value, uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRemoteSetting<uint32_t>("rt.rpc.default.request_timeout");

  rtJsonDocPtr res;
  rtError e = m_stream->sendRequest(req, [&res](rtJsonDocPtr const& doc)
  {
    res = doc;
    return res != nullptr ? RT_OK : RT_FAIL;
  }, timeout);

  if (e != RT_OK)
  {
    rtLogDebug("get failed: %s", rtStrError(e));
    return e;
  }

  if (!res)
  {
    rtLogDebug("get failed to return a response");
    return RT_FAIL;
  }

  rtError statusCode = rtMessage_GetStatusCode(*res);
  if (statusCode != RT_OK)
  {
    return statusCode;
  }

  auto itr = res->FindMember(kFieldNameValue);
  if (itr == res->MemberEnd())
  {
    rtLogWarn("response doesn't contain: %s", kFieldNameValue);
    return RT_FAIL;
  }

  e = rtValueReader::read(value, itr->value, shared_from_this());
  if (e != RT_OK)
  {
    rtLogWarn("failed to read value from response");
    return e;
  }

  return rtMessage_GetStatusCode(*res);
}
  
rtError
rtRemoteClient::set(std::string const& objectName, char const* propertyName, rtValue const& value,
  uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRemoteSetting<uint32_t>("rt.rpc.default.request_timeout");

  rtRemoteSetRequest req(objectName, propertyName);
  req.setValue(value);
  return sendSet(req, timeout);
}

rtError
rtRemoteClient::set(std::string const& objectName, uint32_t propertyIndex, rtValue const& value,
  uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRemoteSetting<uint32_t>("rt.rpc.default.request_timeout");

  rtRemoteSetRequest req(objectName, propertyIndex);
  req.setValue(value);
  return sendSet(req, timeout);
}

rtError
rtRemoteClient::sendSet(rtRemoteSetRequest const& req, uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRemoteSetting<uint32_t>("rt.rpc.default.request_timeout");

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
rtRemoteClient::send(std::string const& objectName, std::string const& methodName,
  int argc, rtValue const* argv, rtValue* result, uint32_t timeout)
{
  if (timeout == 0)
    timeout = rtRemoteSetting<uint32_t>("rt.rpc.default.request_timeout");

  rtRemoteMethodCallRequest req(objectName);
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

  if (result)
  {
    e = rtValueReader::read(*result, itr->value, shared_from_this());
    if (e != RT_OK)
      return e;
  }

  return rtMessage_GetStatusCode(*res);
}
