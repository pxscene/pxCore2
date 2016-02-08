#include "rtRpcTransport.h"
#include "rapidjson/rapidjson.h"

rtError
rtValueWriter::write(rtValue const& from, rapidjson::Value& to, rapidjson::Document& doc)
{
  to.SetObject();
  to.AddMember("type", static_cast<int>(from.getType()), doc.GetAllocator());

  switch (from.getType())
  {
    case RT_voidType:     break;
    case RT_valueType:    assert(false); break;
    case RT_boolType:     to.AddMember("value", from.toBool(), doc.GetAllocator()); break;
    case RT_int8_tType:   to.AddMember("value", from.toInt8(), doc.GetAllocator()); break;
    case RT_uint8_tType:  to.AddMember("value", from.toUInt8(), doc.GetAllocator()); break;
    case RT_floatType:    to.AddMember("value", from.toFloat(), doc.GetAllocator()); break;
    case RT_doubleType:   to.AddMember("value", from.toDouble(), doc.GetAllocator()); break;
    case RT_stringType:   to.AddMember("value", std::string(from.toString().cString()), doc.GetAllocator()); break;
    case RT_objectType:   assert(false); break;
    case RT_functionType: break;
    case RT_voidPtrType:  assert(false); break;
  }
  return RT_OK;
}
