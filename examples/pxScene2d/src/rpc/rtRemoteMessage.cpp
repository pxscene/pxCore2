#include "rtRemoteMessage.h"
#include "rtRemoteClient.h"
#include "rtValueReader.h"
#include "rtValueWriter.h"
#include "rtError.h"

#include <arpa/inet.h>
#include <rtLog.h>
#include <stdarg.h>

#include <atomic>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace
{
  std::atomic<rtCorrelationKey> s_next_key;

  void dumpDoc(rapidjson::Document const& doc, char const* fmt, ...)
  {
    char msg[256];
    memset(msg, 0, sizeof(msg));

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    msg[sizeof(msg) - 1] = '\0';
    va_end(args);

    rapidjson::StringBuffer buff;
    rapidjson::Writer<rapidjson::StringBuffer> writer;
    doc.Accept(writer);
    rtLogInfo("%s\n--- BEGIN DOC ---\n%s\n--- END DOC ---", msg, buff.GetString());
  }
}

rtCorrelationKey
rtMessage_GetNextCorrelationKey()
{
  return ++s_next_key;
}

template<class T> T
from_json(rapidjson::GenericValue<rapidjson::UTF8<> > const& /*v*/)
{
  RT_ASSERT(false);
}

template<> char const*
from_json<char const *>(rapidjson::GenericValue<rapidjson::UTF8<> > const& v)
{
  return v.GetString();
}

template<> uint32_t
from_json<uint32_t>(rapidjson::GenericValue<rapidjson::UTF8<> > const& v)
{
  return v.GetUint();
}

struct rtRemoteMessage::Impl
{
  rapidjson::Document d;
};

rtRemoteMessage::rtRemoteMessage(char const* messageType, std::string const& objectName)
  : m_impl(new Impl())
{
  m_impl->d.SetObject();
  m_impl->d.AddMember(kFieldNameMessageType, std::string(messageType), m_impl->d.GetAllocator());
  if (objectName.size())
    m_impl->d.AddMember(kFieldNameObjectId, objectName, m_impl->d.GetAllocator());
}

rtError
rtRemoteMessage::send(int fd, sockaddr_storage const* dest) const
{
  return rtSendDocument(m_impl->d, fd, dest);
}

rtRemoteRequest::rtRemoteRequest(char const* messageType, std::string const& objectName)
  : rtRemoteMessage(messageType, objectName)
{
  m_correlation_key = rtMessage_GetNextCorrelationKey();
  m_impl->d.AddMember(kFieldNameCorrelationKey, m_correlation_key, m_impl->d.GetAllocator());
}

bool
rtRemoteMessage::isValid() const
{
  return m_impl != nullptr;
}

rtRemoteRequestOpenSession::rtRemoteRequestOpenSession(std::string const& objectName)
  : rtRemoteRequest(kMessageTypeOpenSessionRequest, objectName)
{
}

rtRemoteRequestKeepAlive::rtRemoteRequestKeepAlive()
  : rtRemoteRequest(kMessageTypeKeepAliveRequest, "")
{
}

rtRemoteMethodCallRequest::rtRemoteMethodCallRequest(std::string const& objectName)
  : rtRemoteRequest(kMessageTypeMethodCallRequest, objectName)
{
}

void
rtRemoteMethodCallRequest::setMethodName(std::string const& methodName)
{
  m_impl->d.AddMember(kFieldNameFunctionName, methodName, m_impl->d.GetAllocator());
}

void
rtRemoteMethodCallRequest::addMethodArgument(rtRemoteEnvPtr env, rtValue const& arg)
{
  rapidjson::Value jsonValue;
  // TODO: rtValueWriter should be member of rtRemoteEnvironment
  rtError err = rtValueWriter::write(env, arg, jsonValue, m_impl->d);

  if (err == RT_OK)
  {
    auto itr = m_impl->d.FindMember(kFieldNameFunctionArgs);
    if (itr == m_impl->d.MemberEnd())
    {
      rapidjson::Value args(rapidjson::kArrayType);
      args.PushBack(jsonValue, m_impl->d.GetAllocator());
      m_impl->d.AddMember(kFieldNameFunctionArgs, args, m_impl->d.GetAllocator());
    }
    else
    {
      itr->value.PushBack(jsonValue, m_impl->d.GetAllocator());
    }
  }
}

rtRemoteGetRequest::rtRemoteGetRequest(std::string const& objectName, std::string const& fieldName)
  : rtRemoteRequest(kMessageTypeGetByNameRequest, objectName)
{
  m_impl->d.AddMember(kFieldNamePropertyName, fieldName, m_impl->d.GetAllocator());
}

rtRemoteGetRequest::rtRemoteGetRequest(std::string const& objectName, uint32_t fieldIndex)
  : rtRemoteRequest(kMessageTypeGetByNameRequest, objectName)
{
  m_impl->d.AddMember(kFieldNamePropertyIndex, fieldIndex, m_impl->d.GetAllocator());
}

rtRemoteSetRequest::rtRemoteSetRequest(std::string const& objectName, std::string const& fieldName)
  : rtRemoteRequest(kMessageTypeSetByNameRequest, objectName)
{
  m_impl->d.AddMember(kFieldNamePropertyName, fieldName, m_impl->d.GetAllocator());
}

rtRemoteSetRequest::rtRemoteSetRequest(std::string const& objectName, uint32_t fieldIndex)
  : rtRemoteRequest(kMessageTypeSetByIndexRequest, objectName)
{
  m_impl->d.AddMember(kFieldNamePropertyIndex, fieldIndex, m_impl->d.GetAllocator());
}

rtError
rtRemoteSetRequest::setValue(rtRemoteEnvPtr env, rtValue const& value)
{
  rapidjson::Value jsonValue;
  rtError e = rtValueWriter::write(env, value, jsonValue, m_impl->d);
  if (e != RT_OK)
    return e;
  m_impl->d.AddMember(kFieldNameValue, jsonValue, m_impl->d.GetAllocator());
  return e;
}

void
rtRemoteRequestKeepAlive::addObjectName(std::string const& name)
{
  auto itr = m_impl->d.FindMember(kFieldNameKeepAliveIds);
  if (itr == m_impl->d.MemberEnd())
  {
    rapidjson::Value ids(rapidjson::kArrayType);
    ids.PushBack(rapidjson::Value().SetString(name.c_str(), name.size()), m_impl->d.GetAllocator());
    m_impl->d.AddMember(kFieldNameKeepAliveIds, ids, m_impl->d.GetAllocator());
  }
  else
  {
    itr->value.PushBack(rapidjson::Value().SetString(name.c_str(), name.size()), m_impl->d.GetAllocator());
  }
}

rtRemoteMessage::~rtRemoteMessage()
{
}

char const*
rtRemoteMessage::getMessageType() const
{
  return from_json<char const *>(m_impl->d[kFieldNameMessageType]);
}

char const*
rtRemoteMessage::getObjectName() const
{
  return from_json<char const *>(m_impl->d[kFieldNameObjectId]);
}

rtCorrelationKey
rtRemoteMessage::getCorrelationKey() const
{
  RT_ASSERT(m_correlation_key != 0);
  return m_correlation_key;
}

char const*
rtMessage_GetPropertyName(rapidjson::Document const& doc)
{
  rapidjson::Value::ConstMemberIterator itr = doc.FindMember(kFieldNamePropertyName);
  return itr != doc.MemberEnd() 
    ? itr->value.GetString() 
    : NULL;
}

uint32_t
rtMessage_GetPropertyIndex(rapidjson::Document const& doc)
{
  rapidjson::Value::ConstMemberIterator itr = doc.FindMember(kFieldNamePropertyIndex);
  return itr != doc.MemberEnd() 
    ? itr->value.GetUint()
    : kInvalidPropertyIndex;
}

char const*
rtMessage_GetMessageType(rapidjson::Document const& doc)
{
  rapidjson::Value::ConstMemberIterator itr = doc.FindMember(kFieldNameMessageType);
  return itr != doc.MemberEnd() 
    ? itr->value.GetString()
    : NULL;
}

uint32_t
rtMessage_GetCorrelationKey(rapidjson::Document const& doc)
{
  rapidjson::Value::ConstMemberIterator itr = doc.FindMember(kFieldNameCorrelationKey);
  RT_ASSERT(itr != doc.MemberEnd());
  return itr != doc.MemberEnd() ?
    itr->value.GetUint()
    : kInvalidCorrelationKey;
}

char const*
rtMessage_GetObjectId(rapidjson::Document const& doc)
{
  rapidjson::Value::ConstMemberIterator itr = doc.FindMember(kFieldNameObjectId);
  return itr != doc.MemberEnd() 
    ? itr->value.GetString()
    : NULL;
}

rtError
rtMessage_GetStatusCode(rapidjson::Document const& doc)
{
  rapidjson::Value::ConstMemberIterator itr = doc.FindMember(kFieldNameStatusCode);
  if (itr == doc.MemberEnd())
    dumpDoc(doc, "failed to get status code.");
  RT_ASSERT(itr != doc.MemberEnd());
  return static_cast<rtError>(itr->value.GetInt());
}

char const*
rtMessage_GetStatusMessage(rapidjson::Document const& doc)
{
  rapidjson::Value::ConstMemberIterator itr = doc.FindMember(kFieldNameStatusMessage);
  return itr != doc.MemberEnd() 
    ? itr->value.GetString()
    : NULL;
}

rtError
rtMessage_DumpDocument(rapidjson::Document const& doc, FILE* out)
{
  if (out == nullptr)
    out = stdout;
  rapidjson::StringBuffer buff;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
  doc.Accept(writer);
  fprintf(out, "\n%s\n", buff.GetString());
  return RT_OK;
}

rtError
rtMessage_SetStatus(rapidjson::Document& doc, rtError code)
{
  rtError e = RT_OK;
  doc.AddMember(kFieldNameStatusCode, code, doc.GetAllocator());
  return e;
}

rtError
rtMessage_SetStatus(rapidjson::Document& doc, rtError code, char const* fmt, ...)
{
  rtError e = RT_OK;
  doc.AddMember(kFieldNameStatusCode, code, doc.GetAllocator());

  if (fmt != nullptr)
  {
    char buff[256];
    memset(buff, 0, sizeof(buff));

    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buff, sizeof(buff), fmt, args);
    if (n > 0)
      doc.AddMember(kFieldNameStatusMessage, std::string(buff), doc.GetAllocator());
    va_end(args);
  }
  return e;
}

rtRemoteResponse::rtRemoteResponse(char const* messageType, std::string const& objectName)
  : rtRemoteMessage(messageType, objectName)
{
}

rtError
rtRemoteResponse::getStatusCode() const
{
  rtError e = RT_OK;

  auto itr = m_impl->d.FindMember(kFieldNameStatusCode);
  RT_ASSERT(itr != m_impl->d.MemberEnd());

  if (itr == m_impl->d.MemberEnd())
  {
    rtLogError("failed to find %s in rpc response", kFieldNameStatusCode);
    e = RT_FAIL;
  }
  else
  {
    e = static_cast<rtError>(itr->value.GetInt());
  }

  return e;
}

rtRemoteGetResponse::rtRemoteGetResponse(std::string const& objectName)
  : rtRemoteResponse(kMessageTypeGetByNameResponse, objectName)
{
}
