#include "rtRpcMessage.h"
#include <assert.h>

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

