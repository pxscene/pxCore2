#include "rtRpcTransport.h"
#include "rapidjson/rapidjson.h"

rtError
rtValueWriter::write(rtValue const& val, rapidjson::Document& doc)
{
  rapidjson::Value value(rapidjson::kObjectType);
  value.AddMember("type", static_cast<int>(val.getType()), doc.GetAllocator());
  switch (val.getType())
  {
    case RT_voidType:     break;
    case RT_valueType:    assert(false); break;
    case RT_boolType:     value.AddMember("value", val.toBool(), doc.GetAllocator()); break;
    case RT_int8_tType:   value.AddMember("value", val.toInt8(), doc.GetAllocator()); break;
    case RT_uint8_tType:  value.AddMember("value", val.toUInt8(), doc.GetAllocator()); break;
    case RT_floatType:    value.AddMember("value", val.toFloat(), doc.GetAllocator()); break;
    case RT_doubleType:   value.AddMember("value", val.toDouble(), doc.GetAllocator()); break;
    case RT_stringType:   value.AddMember("value", std::string(val.toString().cString()), doc.GetAllocator()); break;
    case RT_objectType:   assert(false); break;
    case RT_functionType: assert(false); break;
    case RT_voidPtrType:  assert(false); break;
  }
  doc.AddMember("value", value, doc.GetAllocator());
  return RT_OK;
}
