#include "rtValueReader.h"
#include "rtRemoteClient.h"
#include "rtRemoteObject.h"
#include "rtRemoteFunction.h"
#include "rtRemoteMessage.h"

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

// maybe use some type of identifier to indicate remote, with reference
// to transport. I don't like transport being a member of rtRemoteObject
// and rtRemoteFunction.
// either of these should be able simply include a reference to a transport
// with a handle returned by server in json message.

#if 0
static std::string toString(rapidjson::Value const& v)
{
  rapidjson::StringBuffer buff;
  rapidjson::Writer< rapidjson::GenericStringBuffer< rapidjson::UTF8<> > > writer(buff);
  v.Accept(writer);

  char const* s = buff.GetString();
  return (s != nullptr ? std::string(s) : std::string());
}
#endif

rtError
rtValueReader::read(rtValue& to, rapidjson::Value const& from, std::shared_ptr<rtRemoteClient> const& client)
{
  auto type = from.FindMember(kFieldNameValueType);
  if (type  == from.MemberEnd())
  {
    rtLogWarn("failed to find member: %s", kFieldNameValueType);
    return RT_FAIL;
  }

  auto val = from.FindMember(kFieldNameValueValue);
  if (type->value.GetInt() != RT_functionType && val == from.MemberEnd())
  {
    rtLogWarn("failed to find member: %s", kFieldNameValueValue);
    return RT_FAIL;
  }

  switch (type->value.GetInt())
  {
    case RT_voidType:
    to = rtValue();
    break;

    case RT_valueType:
    assert(false);
    break;

    case RT_boolType:
    to = rtValue(val->value.GetBool());
    break;

    case RT_int8_tType:
    to.setInt8(static_cast<int8_t>(val->value.GetInt()));
    break;

    case RT_uint8_tType:
    to.setUInt8(static_cast<uint8_t>(val->value.GetInt()));
    break;

    case RT_int32_tType:
    to.setInt32(static_cast<int32_t>(val->value.GetInt()));
    break;

    case RT_uint32_tType:
    to.setUInt32(static_cast<uint32_t>(val->value.GetUint()));
    break;

    case RT_int64_tType:
    to.setInt64(static_cast<int64_t>(val->value.GetInt64()));
    break;
    
    case RT_uint64_tType:
    to.setUInt64(static_cast<uint64_t>(val->value.GetUint64()));
    break;

    case RT_floatType:
    to.setFloat(static_cast<float>(val->value.GetDouble()));
    break;
    
    case RT_doubleType:
    to.setFloat(static_cast<double>(val->value.GetDouble()));
    break;

    case RT_stringType:
    {
      rtString s(val->value.GetString());
      to.setString(s);
    }
    break;

    case RT_objectType:
    {
      assert(client != NULL);
      if (!client)
        return RT_FAIL;

      auto const& obj = from.FindMember("value");
      assert(obj != from.MemberEnd());

      auto id = obj->value.FindMember(kFieldNameObjectId);
      to.setObject(new rtRemoteObject(id->value.GetString(), client));
    }
    break;

    case RT_functionType:
    {
      assert(client != NULL);
      if (!client)
        return RT_FAIL;

      std::string objectId;
      std::string functionId;

      auto const& func = from.FindMember("value");
      if (func != from.MemberEnd())
      {
        auto itr = func->value.FindMember(kFieldNameObjectId);
        assert(itr != func->value.MemberEnd());
        objectId = itr->value.GetString();

        itr = func->value.FindMember(kFieldNameFunctionName);
        assert(itr != func->value.MemberEnd());
        functionId = itr->value.GetString();
      }
      else
      {
        auto itr = from.FindMember(kFieldNameObjectId);
        assert(itr != from.MemberEnd());
        objectId = itr->value.GetString();

        itr = from.FindMember(kFieldNameFunctionName);
        assert(itr != from.MemberEnd());
        functionId = itr->value.GetString();
      }
	
      to.setFunction(new rtRemoteFunction(objectId, functionId, client));
    }
    break;

    case RT_voidPtrType:
    {
#if __x86_64
      to.setVoidPtr((void *) val->value.GetUint64());
#else
      to.setVoidPtr((void *) val->value.GetUint());
#endif
    }
    break;
  }

  return RT_OK;
}
