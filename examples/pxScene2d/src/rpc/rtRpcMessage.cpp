#include "rtRpcMessage.h"

#include <arpa/inet.h>
#include <rtLog.h>
#include <assert.h>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

template<class T> rtValue
from_json(rapidjson::GenericValue<rapidjson::UTF8<> > const& /*v*/) { assert(false); }

template<> rtValue
from_json<rtString>(rapidjson::GenericValue<rapidjson::UTF8<> > const& v) { return rtValue(v.GetString()); }

template<> rtValue
from_json<uint32_t>(rapidjson::GenericValue<rapidjson::UTF8<> > const& v) { return rtValue(v.GetUint()); }

template<class T> rtError
get_property(rapidjson::Document const& doc, char const* n, rtValue& v)
{
  rtError e = RT_FAIL;
  if (n)
  {
    auto itr = doc.FindMember(n);
    if (itr != doc.MemberEnd())
    {
      v = rtValue(from_json<T>(itr->value));
      e = RT_OK;
    }
  }
  return e;
}

struct rtRpcMessage::Impl
{
  rapidjson::Document d;
};

bool
rtRpcMessage::isValid() const
{
  return m_impl != NULL;
}

rtRpcMessage::rtRpcMessage()
  : m_impl(new Impl())
{
  m_impl->d.SetObject();
}

rtRpcMessage::~rtRpcMessage()
{
}

rtError
rtRpcMessage::getPropertyName(rtValue& v)
{
  return get_property<rtString>(m_impl->d, kFieldNamePropertyName, v);
}

rtError
rtRpcMessage::getPropertyIndex(rtValue& v)
{
  return get_property<uint32_t>(m_impl->d, kFieldNamePropertyName, v);
}

rtError
rtRpcMessage::getMessageType(rtValue& v) const
{
  return get_property<rtString>(m_impl->d, kFieldNameMessageType, v);
}

rtError
rtRpcMessage::getCorrelationKey(rtValue& v)
{
  return get_property<uint32_t>(m_impl->d, kFieldNameCorrelationKey, v);
}

rtError
rtRpcMessage::getObjectId(rtValue& v)
{
  return get_property<rtString>(m_impl->d, kFieldNameObjectId, v);
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

