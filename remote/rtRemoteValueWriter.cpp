/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "rtRemoteValueWriter.h"
#include "rtRemoteObject.h"
#include "rtRemoteFunction.h"
#include "rtRemoteObjectCache.h"
#include "rtRemoteConfig.h"
#include "rtRemoteMessage.h"
#include "rtRemoteEnvironment.h"
#include "rtGuid.h"

#include <rtObject.h>
#include <rapidjson/rapidjson.h>
#include <sstream>

namespace
{
//  static std::atomic<int> sNextFunctionId;

  std::string getId(rtFunctionRef const& ref)
  {
    rtFunctionCallback * obj = dynamic_cast<rtFunctionCallback *>(ref.ptr());
    if(obj && !obj->getId().empty())
    {
      return obj->getId();
    }
    else
    {
      rtGuid id = rtGuid::newRandom();

      std::stringstream ss;
      ss << "func://";
      ss << id.toString();

      if(obj)
        obj->setId(ss.str());

      return ss.str();
    }
  }
 
  std::string getId(rtObjectRef const& ref)
  {
    rtRemoteObject const* obj = dynamic_cast<rtRemoteObject const *>(ref.getPtr());
    if (obj != nullptr)
    {
      return obj->getId();
    }
    else
    {
      rtGuid id = rtGuid::newRandom();

      std::stringstream ss;
      ss << "obj://";
      ss << id.toString();
      return ss.str();
    }
  }
}

rtError
rtRemoteValueWriter::write(rtRemoteEnvironment* env, rtValue const& from,
  rapidjson::Value& to, rapidjson::Document& doc)
{
  to.SetObject();

  if (from.getType() == RT_functionType)
  {
    to.AddMember(kFieldNameValueType, static_cast<int>(RT_functionType), doc.GetAllocator());

    rtFunctionRef func = from.toFunction();
    std::string id = func ? getId(func) : kNullObjectId;

    rapidjson::Value val;
    val.SetObject();
    val.AddMember(kFieldNameObjectId, std::string("global"), doc.GetAllocator());
    val.AddMember(kFieldNameFunctionName, id, doc.GetAllocator());
    to.AddMember("value", val, doc.GetAllocator());

    if (func)
    {
      env->ObjectCache->insert(id, func);
    }

    return RT_OK;
  }

  if (from.getType() == RT_objectType)
  {
    to.AddMember(kFieldNameValueType, static_cast<int>(RT_objectType), doc.GetAllocator());

    rtObjectRef obj = from.toObject();
    std::string id = obj ? getId(obj) : kNullObjectId;

    rapidjson::Value val;
    val.SetObject();
    val.AddMember(kFieldNameObjectId, id, doc.GetAllocator());
    to.AddMember("value", val, doc.GetAllocator());

    if (obj)
    {
      env->ObjectCache->insert(id, obj);
    }

    return RT_OK;
  }

  to.AddMember("type", static_cast<int>(from.getType()), doc.GetAllocator());

  switch (from.getType())
  {
    case RT_voidType:     break;
    case RT_valueType:    RT_ASSERT(false); break;
    case RT_boolType:     to.AddMember("value", from.toBool(), doc.GetAllocator()); break;
    case RT_int8_tType:   to.AddMember("value", from.toInt8(), doc.GetAllocator()); break;
    case RT_uint8_tType:  to.AddMember("value", from.toUInt8(), doc.GetAllocator()); break;
    case RT_floatType:    to.AddMember("value", from.toFloat(), doc.GetAllocator()); break;
    case RT_doubleType:   to.AddMember("value", from.toDouble(), doc.GetAllocator()); break;
    case RT_int32_tType:  to.AddMember("value", from.toInt32(), doc.GetAllocator()); break;
    case RT_uint32_tType: to.AddMember("value", from.toUInt32(), doc.GetAllocator()); break;
    case RT_int64_tType:  to.AddMember("value", from.toInt64(), doc.GetAllocator()); break;
    case RT_uint64_tType: to.AddMember("value", from.toUInt64(), doc.GetAllocator()); break;
    case RT_stringType:   to.AddMember("value", std::string(from.toString().cString()), doc.GetAllocator()); break;
    case RT_voidPtrType:
#if __x86_64
      to.AddMember("value", (uint64_t)(from.toVoidPtr()), doc.GetAllocator());
#else
      to.AddMember("value", (uint32_t)(from.toVoidPtr()), doc.GetAllocator());
#endif
      break;

    default:
      rtLogWarn("invalid type: %d", static_cast<int>(from.getType()));
      RT_ASSERT(false);
      break;
  }
  return RT_OK;
}
