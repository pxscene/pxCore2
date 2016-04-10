#include "rtRpcMessage.h"
#include "rtRpcClient.h"
#include "rtValueReader.h"
#include "rtValueWriter.h"

#include <arpa/inet.h>
#include <rtLog.h>
#include <assert.h>
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
  std::atomic<rtCorrelationKey_t> s_next_key;
}

rtCorrelationKey_t
rtMessage_GetNextCorrelationKey()
{
  return ++s_next_key;
}

template<class T> T
from_json(rapidjson::GenericValue<rapidjson::UTF8<> > const& /*v*/)
{
  assert(false);
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

struct rtRpcMessage::Impl
{
  rapidjson::Document d;
};

rtRpcMessage::rtRpcMessage(char const* messageType, std::string const& objectName)
  : m_impl(new Impl())
{
  m_impl->d.SetObject();
  m_impl->d.AddMember(kFieldNameMessageType, std::string(messageType), m_impl->d.GetAllocator());
  if (objectName.size())
    m_impl->d.AddMember(kFieldNameObjectId, objectName, m_impl->d.GetAllocator());
}

rtError
rtRpcMessage::send(int fd, sockaddr_storage const* dest) const
{
  return rtSendDocument(m_impl->d, fd, dest);
}

rtRpcRequest::rtRpcRequest(char const* messageType, std::string const& objectName)
  : rtRpcMessage(messageType, objectName)
{
  m_correlation_key = rtMessage_GetNextCorrelationKey();
  m_impl->d.AddMember(kFieldNameCorrelationKey, m_correlation_key, m_impl->d.GetAllocator());
}

bool
rtRpcMessage::isValid() const
{
  return m_impl != nullptr;
}

rtRpcRequestOpenSession::rtRpcRequestOpenSession(std::string const& objectName)
  : rtRpcRequest(kMessageTypeOpenSessionRequest, objectName)
{
}

rtRpcRequestKeepAlive::rtRpcRequestKeepAlive()
  : rtRpcRequest(kMessageTypeKeepAliveRequest, "")
{
}

rtRpcMethodCallRequest::rtRpcMethodCallRequest(std::string const& objectName)
  : rtRpcRequest(kMessageTypeMethodCallRequest, objectName)
{
}

void
rtRpcMethodCallRequest::setMethodName(std::string const& methodName)
{
  m_impl->d.AddMember(kFieldNameFunctionName, methodName, m_impl->d.GetAllocator());
}

void
rtRpcMethodCallRequest::addMethodArgument(rtValue const& arg)
{
  rapidjson::Value jsonValue;
  rtError err = rtValueWriter::write(arg, jsonValue, m_impl->d);

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

rtRpcGetRequest::rtRpcGetRequest(std::string const& objectName, std::string const& fieldName)
  : rtRpcRequest(kMessageTypeGetByNameRequest, objectName)
{
  m_impl->d.AddMember(kFieldNamePropertyName, fieldName, m_impl->d.GetAllocator());
}

rtRpcGetRequest::rtRpcGetRequest(std::string const& objectName, uint32_t fieldIndex)
  : rtRpcRequest(kMessageTypeGetByNameRequest, objectName)
{
  m_impl->d.AddMember(kFieldNamePropertyIndex, fieldIndex, m_impl->d.GetAllocator());
}

rtRpcSetRequest::rtRpcSetRequest(std::string const& objectName, std::string const& fieldName)
  : rtRpcRequest(kMessageTypeSetByNameRequest, objectName)
{
  m_impl->d.AddMember(kFieldNamePropertyName, fieldName, m_impl->d.GetAllocator());
}

rtRpcSetRequest::rtRpcSetRequest(std::string const& objectName, uint32_t fieldIndex)
  : rtRpcRequest(kMessageTypeSetByIndexRequest, objectName)
{
  m_impl->d.AddMember(kFieldNamePropertyIndex, fieldIndex, m_impl->d.GetAllocator());
}

rtError
rtRpcSetRequest::setValue(rtValue const& value)
{
  rapidjson::Value jsonValue;
  rtError e = rtValueWriter::write(value, jsonValue, m_impl->d);
  if (e != RT_OK)
    return e;
  m_impl->d.AddMember(kFieldNameValue, jsonValue, m_impl->d.GetAllocator());
  return e;
}

void
rtRpcRequestKeepAlive::addObjectName(std::string const& name)
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

rtRpcMessage::~rtRpcMessage()
{
}

char const*
rtRpcMessage::getMessageType() const
{
  return from_json<char const *>(m_impl->d[kFieldNameMessageType]);
}

char const*
rtRpcMessage::getObjectName() const
{
  return from_json<char const *>(m_impl->d[kFieldNameObjectId]);
}

rtCorrelationKey_t
rtRpcMessage::getCorrelationKey() const
{
  assert(m_correlation_key != 0);
  return m_correlation_key;
  // return from_json<uint32_t>(m_impl->d[kFieldNameCorrelationKey]);
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
  rapidjson::Value::ConstMemberIterator itr = doc.FindMember(kFieldNamePropertyName);
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
  assert(itr != doc.MemberEnd());
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
  assert(itr != doc.MemberEnd());
  return static_cast<rtError>(itr->value.GetInt());
}

#if 0
rtError
rtRpcMessage::readMessage(int fd, rt_sockbuf_t& buff, rtRpcMessage& m)
{
  rtError err = RT_OK;

  int n = 0;
  int capacity = static_cast<int>(buff.capacity());

  err = rtReadUntil(fd, reinterpret_cast<char *>(&n), 4);
  if (err != RT_OK)
  {
    rtLogWarn("error reading length from socket");
    return err;
  }

  n = ntohl(n);

  if (n > capacity)
  {
    rtLogWarn("buffer capacity %d not big enough for message size: %d", capacity, n);
    // TODO: should drain, and discard message
    assert(false);
    return RT_FAIL;
  }

  buff.resize(n + 1);
  buff[n] = '\0';

  err = rtReadUntil(fd, &buff[0], n);
  if (err != RT_OK)
  {
    rtLogError("failed to read payload message of length %d from socket", n);
    return err;
  }

  #ifdef RT_RPC_DEBUG
  rtLogDebug("read (%d):\n\t\"%.*s\"\n", static_cast<int>(buff.size()), static_cast<int>(buff.size()), &buff[0]);
  #endif

  rapidjson::MemoryStream stream(&buff[0], n);
  if (m.m_impl->d.ParseStream<rapidjson::kParseDefaultFlags>(stream).HasParseError())
  {
    int begin = m.m_impl->d.GetErrorOffset() - 16;
    if (begin < 0)
      begin = 0;
    int end = begin + 64;
    if (end > n)
      end = n;
    int length = (end - begin);

    rtLogWarn("unparsable JSON read:%d offset:%d", m.m_impl->d.GetParseError(), (int) m.m_impl->d.GetErrorOffset());
    rtLogWarn("\"%.*s\"\n", length, &buff[0] + begin);

    return RT_FAIL;
  }

  return RT_OK;
}
#endif

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

rtRpcResponse::rtRpcResponse(char const* messageType, std::string const& objectName)
  : rtRpcMessage(messageType, objectName)
{
}

rtError
rtRpcResponse::getStatusCode() const
{
  auto itr = m_impl->d.FindMember(kFieldNameStatusCode);
  if (itr == m_impl->d.MemberEnd())
  {
    rtLogError("failed to find %s in rpc response", kFieldNameStatusCode);
    assert(false);
    return RT_FAIL;
  }
  return static_cast<rtError>(itr->value.GetInt());
}

rtRpcGetResponse::rtRpcGetResponse(std::string const& objectName)
  : rtRpcResponse(kMessageTypeGetByNameResponse, objectName)
{
}
