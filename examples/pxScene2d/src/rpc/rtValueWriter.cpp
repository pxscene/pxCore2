#ifndef __RT_RPC_MESSAGE_H__
#define __RT_RPC_MESSAGE_H__

#include "rtLog.h"
#include "rtSocketUtils.h"
#include "rtRpcConfig.h"

#include <cstdio>
#include <limits>
#include <rtError.h>
#include <rtValue.h>
#include <rapidjson/document.h>

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

class rtRpcMessage
{
public:
  virtual ~rtRpcMessage();
  bool isValid() const;

protected:
  rtRpcMessage(char const* messageType, std::string const& objectName);

private:
  rtRpcMessage() { }

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

class rtRpcRequest : public rtRpcMessage
{
protected:
  rtRpcRequest(char const* messageType, std::string const& objectName);
};

class rtRpcResponse : public rtRpcMessage
{
public:
  rtRpcResponse(char const* messageType, std::string const& objectName);
public:
  rtError getStatusCode() const;
  inline bool isValid() const
    { return m_is_valid; }
private:
  bool m_is_valid;
};

class rtRpcGetResponse : public rtRpcResponse
{
public:
  rtRpcGetResponse(std::string const& objectName);
  rtValue getValue() const;
};


class rtRpcRequestOpenSession : public rtRpcRequest
{
public:
  rtRpcRequestOpenSession(std::string const& objectName);
};

class rtRpcRequestKeepAlive : public rtRpcRequest
{
public:
  rtRpcRequestKeepAlive();
  void addObjectName(std::string const& name);
};

class rtRpcMethodCallRequest : public rtRpcRequest
{
public:
  rtRpcMethodCallRequest(std::string const& objectName);
  void setMethodName(std::string const& methodName);
  void addMethodArgument(rtValue const& arg);
};

class rtRpcGetRequest : public rtRpcRequest
{
public:
  rtRpcGetRequest(std::string const& objectName, std::string const& fieldName);
  rtRpcGetRequest(std::string const& objectName, uint32_t fieldIndex);
};

class rtRpcSetRequest : public rtRpcRequest
{
public:
  rtRpcSetRequest(std::string const& objectName, std::string const& fieldName);
  rtRpcSetRequest(std::string const& objectName, uint32_t fieldIndex);
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

#include "rtValueWriter.h"
#include "rtObjectCache.h"
#include "rtRpcMessage.h"

#include <rtObject.h>
#include <rapidjson/rapidjson.h>

#if 0
rapidjson::Value val;
if (value.getType() == RT_functionType)
{
  val.SetObject();
  val.AddMember(kFieldNameObjectId, std::string(id),
      res->GetAllocator());
  if (name)
    val.AddMember(kFieldNameFunctionName, std::string(name), res->GetAllocator());
  else
    val.AddMember(kFieldNameFunctionIndex, index, res->GetAllocator());
  val.AddMember(kFieldNameValueType, static_cast<int>(RT_functionType), res->GetAllocator());
}
#endif


namespace
{
  std::string getId(rtFunctionRef const& ref)
  {
    char buff[64] = {0};
    std::snprintf(buff, sizeof(buff), "%p", ref.getPtr());
    return std::string(buff);
  }
}

rtError
rtValueWriter::write(rtValue const& from, rapidjson::Value& to, rapidjson::Document& doc)
{
  to.SetObject();

  if (from.getType() == RT_functionType)
  {
    rtFunctionRef func = from.toFunction();
    std::string id = getId(func);

    rapidjson::Value val;
    val.SetObject();
    val.AddMember(kFieldNameObjectId, std::string("global"), doc.GetAllocator());
    val.AddMember(kFieldNameFunctionName, id, doc.GetAllocator());
    val.AddMember(kFieldNameValueType, static_cast<int>(RT_functionType), doc.GetAllocator());

    rtObjectCache::insert(id, func, rtRpcSetting<int>("rt.rpc.cache.max_object_lifetime"));
    to.AddMember("value", val, doc.GetAllocator());

    return RT_OK;
  }

  to.AddMember("type", static_cast<int>(from.getType()), doc.GetAllocator());

  switch (from.getType())
  {
    case RT_voidType:     break;
    case RT_valueType:    assert(false); break;
    case RT_boolType:     to.AddMember("value", from.toBool(), doc.GetAllocator()); break;
    case RT_int8_tType:   to.AddMember("value", from.toInt8(), doc.GetAllocator()); break;
    case RT_uint8_tType:  to.AddMember("value", from.toUInt8(), doc.GetAllocator()); break;
    case RT_floatType:    to.AddMember("value", from.toFloat(), doc.GetAllocator()); break;
    case RT_doubleType:   to.AddMember("value", from.toDouble(), doc.GetAllocator()); break;
    case RT_int32_tType:  to.AddMember("value", from.toInt32(), doc.GetAllocator()); break;
    case RT_uint32_tType: to.AddMember("value", from.toUInt32(), doc.GetAllocator()); break;
    case RT_stringType:   to.AddMember("value", std::string(from.toString().cString()), doc.GetAllocator()); break;
    case RT_objectType:   assert(false); break;
    case RT_functionType: assert(false); break;
    case RT_voidPtrType:  assert(false); break;
  }
  return RT_OK;
}
