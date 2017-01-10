#ifndef __RT_REMOTE_MESSAGE_H__
#define __RT_REMOTE_MESSAGE_H__

#include <limits>
#include <memory>
#include <rapidjson/document.h>
#include <rtError.h>
#include <rtValue.h>

#include "rtLog.h"
#include "rtRemoteCorrelationKey.h"

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
#define kFieldNamePath "path"
#define kFieldNameScheme "scheme"
#define kFieldNameEndpointType "endpoint.type"
#define kFieldNameReplyTo "reply-to"
#define kEndpointTypeLocal "local.endpoint"
#define kEndpointTypeRemote "net.endpoint"

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

#ifdef RT_REMOTE_CORRELATION_KEY_IS_INT
#define kInvalidCorrelationKey std::numeric_limits<uint32_t>::max()
#else
#define kInvalidCorrelationKey rtGuid::null()
#endif

#define kNsMessageTypeLookup "ns.lookup"
#define kNsMessageTypeLookupResponse "ns.lookup.response"
#define kNsMessageTypeDeregister "ns.deregister"
#define kNsMessageTypeDeregisterResponse "ns.deregister.response"
#define kNsMessageTypeUpdate "ns.update"
#define kNsMessageTypeUpdateResponse "ns.update.response"
#define kNsMessageTypeRegister "ns.register"
#define kNsMessageTypeRegisterResponse "ns.register.response"
#define kNsFieldNameStatusCode "ns.status"
#define kNsStatusSuccess "ns.status.success"
#define kNsStatusFail "ns.status.fail"

using rtRemoteMessage     = rapidjson::Document;
using rtRemoteMessagePtr  = std::shared_ptr<rtRemoteMessage>;

char const*             rtMessage_GetPropertyName(rtRemoteMessage const& m);
uint32_t                rtMessage_GetPropertyIndex(rtRemoteMessage const& m);
char const*             rtMessage_GetMessageType(rtRemoteMessage const& m);
rtRemoteCorrelationKey  rtMessage_GetCorrelationKey(rtRemoteMessage const& m);
char const*             rtMessage_GetObjectId(rtRemoteMessage const& m);
rtError                 rtMessage_GetStatusCode(rtRemoteMessage const& m);
char const*             rtMessage_GetStatusMessage(rtRemoteMessage const& m);
rtError                 rtMessage_Dump(rtRemoteMessage const& m, FILE* out = stdout);
rtError                 rtMessage_SetStatus(rtRemoteMessage& m, rtError code, char const* fmt, ...) RT_PRINTF_FORMAT(3, 4);
rtError                 rtMessage_SetStatus(rtRemoteMessage& m, rtError code);
rtRemoteCorrelationKey  rtMessage_GetNextCorrelationKey();

#endif
