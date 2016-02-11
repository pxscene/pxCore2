#include "rtRpcTransport.h"
#include "rtRemoteObject.h"
#include "rtRemoteFunction.h"
#include "rtRpcMessage.h"

rtError
rtValueReader::read(rtValue& to, rapidjson::Value const& from, std::shared_ptr<rtRpcTransport> const& tport)
{
  auto type = from.FindMember(kFieldNameValueType);
  if (type  == from.MemberEnd())
    return RT_FAIL;

  auto val = from.FindMember(kFieldNameValueValue);
  if (type->value.GetInt() != RT_functionType && val == from.MemberEnd())
    return RT_FAIL;

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
      assert(tport != NULL);
      if (!tport)
        return RT_FAIL;

      auto id = from.FindMember(kFieldNameObjectId);
      if (id == from.MemberEnd())
        return RT_FAIL;
      to.setObject(new rtRemoteObject(id->value.GetString(), tport));
    }
    break;

    case RT_functionType:
    {
      assert(tport != NULL);
      if (!tport)
        return RT_FAIL;

      auto id = from.FindMember(kFieldNameObjectId);
      if (id == from.MemberEnd())
      {
        rtLogWarn("function doesn't have field: %s", kFieldNameObjectId);
        return RT_FAIL;
      }
      auto name = from.FindMember(kFieldNameFunctionName);
      if (name == from.MemberEnd())
      {
        rtLogWarn("function doesn't have field: %s", kFieldNameObjectId);
        return RT_FAIL;
      }
      to.setFunction(new rtRemoteFunction(id->value.GetString(), name->value.GetString(), tport));
    }
    break;

    case RT_voidPtrType:
    assert(false);
    break;
  }

  return RT_OK;
}
