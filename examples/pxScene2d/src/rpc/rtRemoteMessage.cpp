#include "rtRemoteMessage.h"
#include "rtRemoteClient.h"
#include "rtValueReader.h"
#include "rtValueWriter.h"
#include "rtError.h"

#include <arpa/inet.h>
#include <rtLog.h>
#include <stdarg.h>
#include <unistd.h>

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
  static pid_t pid = getpid();
  int p = pid;
  p <<= 16;
  p &= 0xffff0000;
  int k = ++s_next_key;
  p |= k;
  return p;
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

rtCorrelationKey
rtMessage_GetCorrelationKey(rapidjson::Document const& doc)
{
  rtCorrelationKey k = kInvalidCorrelationKey;
  rapidjson::Value::ConstMemberIterator itr = doc.FindMember(kFieldNameCorrelationKey);
  RT_ASSERT(itr != doc.MemberEnd());
  if (itr != doc.MemberEnd())
    k = itr->value.GetUint();
  return k;
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
