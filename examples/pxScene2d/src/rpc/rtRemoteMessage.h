#ifndef __RT_RPC_MESSAGE_H__
#define __RT_RPC_MESSAGE_H__

#include <limits>
#include <rapidjson/document.h>
#include <rtError.h>
#include <rtValue.h>
#include "rtLog.h"
#include "rtSocketUtils.h"

#define kFieldNameMessageType "message.type"
#define kFieldNameCorrelationKey "correlation.key"
#define kFieldNameObjectId "object.id"
#define kFieldNamePropertyName "property.name"
#define kFieldNamePropertyIndex "property.index"
#define kFieldNameStatusCode "status.code"
#define kFieldNameStatusMessage "status.message"
#define kFieldNameFunctionName "function.name"
#define kFieldNameFunctionIndex "function.index"
#define kFieldNameFunctionArgs "function.args"
#define kFieldNameFunctionReturn "function.return_value"
#define kFieldNameValue "value"
#define kFieldNameValueType "type"
#define kFieldNameValueValue "value"
#define kFieldNameSenderId "sender.id"
#define kFieldNameKeepAliveIds "keep_alive.ids"
#define kFieldNameIp "ip"
#define kFieldNamePort "port"

#define kMessageTypeInvalidResponse "invalid.response"
#define kMessageTypeSetByNameRequest "set.byname.request"
#define kMessageTypeSetByNameResponse "set.byname.response"
#define kMessageTypeSetByIndexRequest "set.byindex.request"
#define kMessageTypeSetByIndexResponse "set.byindex.response"
#define kMessageTypeGetByNameRequest "get.byname.request"
#define kMessageTypeGetByNameResponse "get.byname.response"
#define kMessageTypeGetByIndexRequest "get.byindex.request"
#define kMessageTypeGetByIndexResponse "get.byindex.response"
#define kMessageTypeOpenSessionResponse "session.open.response"
#define kMessageTypeMethodCallResponse "method.call.response"
#define kMessageTypeKeepAliveResponse "keep_alive.response"
#define kMessageTypeSearch "search"
#define kMessageTypeLocate "locate"
#define kMessageTypeMethodCallRequest "method.call.request"
#define kMessageTypeKeepAliveRequest "keep_alive.request"
#define kMessageTypeOpenSessionRequest "session.open.request"

#define kInvalidPropertyIndex std::numeric_limits<uint32_t>::max()
#define kInvalidCorrelationKey std::numeric_limits<uint32_t>::max()

class rtRemoteMessage
{
public:
  virtual ~rtRemoteMessage();
  bool isValid() const;

protected:
  rtRemoteMessage(char const* messageType, std::string const& objectName);

private:
  rtRemoteMessage() { }

public:
  rtCorrelationKey getCorrelationKey() const;
  char const* getMessageType() const;
  char const* getObjectName() const;

  rtError send(int fd, sockaddr_storage const* dest) const;

protected:
  struct Impl;
  std::shared_ptr<Impl>   m_impl;
  rtCorrelationKey      m_correlation_key;
};

class rtRemoteRequest : public rtRemoteMessage
{
protected:
  rtRemoteRequest(char const* messageType, std::string const& objectName);
};

class rtRemoteResponse : public rtRemoteMessage
{
public:
  rtRemoteResponse(char const* messageType, std::string const& objectName);
public:
  rtError getStatusCode() const;
  inline bool isValid() const
    { return m_is_valid; }
private:
  bool m_is_valid;
};

class rtRemoteGetResponse : public rtRemoteResponse
{
public:
  rtRemoteGetResponse(std::string const& objectName);
  rtValue getValue() const;
};


class rtRemoteRequestOpenSession : public rtRemoteRequest
{
public:
  rtRemoteRequestOpenSession(std::string const& objectName);
};

class rtRemoteRequestKeepAlive : public rtRemoteRequest
{
public:
  rtRemoteRequestKeepAlive();
  void addObjectName(std::string const& name);
};

class rtRemoteMethodCallRequest : public rtRemoteRequest
{
public:
  rtRemoteMethodCallRequest(std::string const& objectName);
  void setMethodName(std::string const& methodName);
  void addMethodArgument(rtValue const& arg);
};

class rtRemoteGetRequest : public rtRemoteRequest
{
public:
  rtRemoteGetRequest(std::string const& objectName, std::string const& fieldName);
  rtRemoteGetRequest(std::string const& objectName, uint32_t fieldIndex);
};

class rtRemoteSetRequest : public rtRemoteRequest
{
public:
  rtRemoteSetRequest(std::string const& objectName, std::string const& fieldName);
  rtRemoteSetRequest(std::string const& objectName, uint32_t fieldIndex);
  rtError setValue(rtValue const& value);
};

char const* rtMessage_GetPropertyName(rapidjson::Document const& doc);
uint32_t    rtMessage_GetPropertyIndex(rapidjson::Document const& doc);
char const* rtMessage_GetMessageType(rapidjson::Document const& doc);
uint32_t    rtMessage_GetCorrelationKey(rapidjson::Document const& doc);
char const* rtMessage_GetObjectId(rapidjson::Document const& doc);
rtError     rtMessage_GetStatusCode(rapidjson::Document const& doc);
rtError     rtMessage_DumpDocument(rapidjson::Document const& doc, FILE* out = stdout);
rtError     rtMessage_SetStatus(rapidjson::Document& doc, rtError code, char const* fmt, ...)
              RT_PRINTF_FORMAT(3, 4);
rtError     rtMessage_SetStatus(rapidjson::Document& doc, rtError code);
rtCorrelationKey rtMessage_GetNextCorrelationKey();


#endif
