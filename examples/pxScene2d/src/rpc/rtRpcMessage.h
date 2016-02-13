#ifndef __RT_RPC_MESSAGE_H__
#define __RT_RPC_MESSAGE_H__

#include <limits>
#include <rapidjson/document.h>
#include <rtError.h>
#include <rtValue.h>
#include "rtLog.h"
#include "rtSocketUtils.h"

class rtRpcMessage
{
public:
  rtRpcMessage();
  virtual ~rtRpcMessage();

  bool isValid() const;

public:
  rtError getPropertyName(rtValue& v);
  rtError getPropertyIndex(rtValue& v);
  rtError getMessageType(rtValue& v) const;
  rtError getCorrelationKey(rtValue& v);
  rtError getObjectId(rtValue& v);

public:
  static rtError readMessage(int fd, rt_sockbuf_t& buff, rtRpcMessage& m);

private:
  struct Impl;
  std::shared_ptr<Impl> m_impl;
};

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

#define kMessageTypeSetByNameRequest "set.byname.request"
#define kMessageTypeSetByNameResponse "set.byname.response"
#define kMessageTypeSetByIndexRequest "set.byindex.request"
#define kMessageTypeSetByIndexResponse "set.byindex.response"
#define kMessageTypeGetByNameRequest "get.byname.request"
#define kMessageTypeGetByNameResponse "get.byname.response"
#define kMessageTypeGetByIndexRequest "get.byindex.request"
#define kMessageTypeGetByIndexResponse "get.byindex.response"
#define kMessageTypeOpenSessionRequest "session.open.request"
#define kMessageTypeOpenSessionResponse "session.open.response"
#define kMessageTypeMethodCallResponse "method.call.response"
#define kMessageTypeMethodCallRequest "method.call.request"
#define kMessageTypeKeepAliveRequest "keep_alive.request"

#define kInvalidPropertyIndex std::numeric_limits<uint32_t>::max()
#define kInvalidCorrelationKey std::numeric_limits<uint32_t>::max()

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


#endif
