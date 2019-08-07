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

#include "rtJsonUtils.h"

#include "rtObject.h"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>

// write
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

namespace
{
  const int FILE_BUFFER_SIZE = 65536;

  rtError jsonValue2rtValue(const rapidjson::Value& jsonValue, rtValue& v)
  {
    // json array
    if (jsonValue.IsArray())
    {
      rtArrayObject* o = new rtArrayObject;
      for (rapidjson::SizeType k = 0; k < jsonValue.Size(); k++)
      {
        const rapidjson::Value& val = jsonValue[k];
        rtValue val2;
        if (jsonValue2rtValue(val, val2) == RT_OK)
          o->pushBack(val2);
      }
      v = o;
    }

    // json object
    else if (jsonValue.IsObject())
    {
      rtMapObject* o = new rtMapObject;
      for (rapidjson::Value::ConstMemberIterator itr = jsonValue.MemberBegin(); itr != jsonValue.MemberEnd(); ++itr)
      {
        const rapidjson::Value& key = itr->name;
        const rapidjson::Value& val = itr->value;
        if (!key.IsString())
        {
          rtLogError("%s : map key is not string", __FUNCTION__);
          continue;
        }
        rtValue val2;
        if (jsonValue2rtValue(val, val2) == RT_OK)
          o->Set(key.GetString(), &val2);
      }
      v = o;
    }

    // json string
    else if (jsonValue.IsString())
      v = jsonValue.GetString();

    // json number
    else if (jsonValue.IsUint())
      v = jsonValue.GetUint();
    else if (jsonValue.IsInt())
      v = jsonValue.GetInt();
    else if (jsonValue.IsInt64())
      v = jsonValue.GetInt64();
    else if (jsonValue.IsUint64())
      v = jsonValue.GetUint64();
    else if (jsonValue.IsDouble())
      v = jsonValue.GetDouble();

    // other types
    else if (jsonValue.IsBool())
      v = jsonValue.GetBool();
    else if (jsonValue.IsNull())
      v.setEmpty();

    else
    {
      rtLogError("%s : value type is not supported", __FUNCTION__);
      return RT_ERROR_NOT_IMPLEMENTED;
    }

    return RT_OK;
  }

  rtError rtValue2jsonValue(const rtValue& v, rapidjson::Document& doc, rapidjson::Value& jsonValue)
  {
    if (v.isEmpty())
      jsonValue.SetNull();
    else
    {
      rtType t = v.getType();
      if (t == RT_boolType)
        jsonValue = v.toBool();
      else if (t == RT_int8_tType)
        jsonValue = v.toInt8();
      else if (t == RT_uint8_tType)
        jsonValue = v.toUInt8();
      else if (t == RT_intType || t == RT_int32_tType)
        jsonValue = v.toInt32();
      else if (t == RT_uint32_tType)
        jsonValue = v.toUInt32();
      else if (t == RT_int64_tType)
        jsonValue = v.toInt64();
      else if (t == RT_uint64_tType)
        jsonValue = v.toUInt64();
      else if (t == RT_floatType)
        jsonValue = v.toFloat();
      else if (t == RT_doubleType)
        jsonValue = v.toDouble();
      else if (t == RT_stringType || t == RT_rtStringType)
      {
        rtString s = v.toString();
        jsonValue.SetString(s.cString(), s.byteLength(), doc.GetAllocator());
      }
      else
      {
        rtLogWarn("%s : unsupported value type: %c", __FUNCTION__, t);
        return RT_ERROR_NOT_IMPLEMENTED;
      }
    }

    return RT_OK;
  }
}

rtError json2rtValue(const char* json, rtValue& val)
{
  if (!json || *json == 0)
  {
    rtLogError("%s : empty", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  rapidjson::Document doc;
  rapidjson::ParseResult result = doc.Parse(json);

  if (!result)
  {
    rapidjson::ParseErrorCode e = doc.GetParseError();
    rtLogError("%s : [JSON parse error : %s (%ld)]", __FUNCTION__, rapidjson::GetParseError_En(e), result.Offset());
    return RT_ERROR;
  }

  return jsonValue2rtValue(doc, val);
}

rtError jsonFile2rtValue(const char* path, rtValue& val)
{
  if (!path || *path == 0)
  {
    rtLogError("%s : empty", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  FILE* fp = fopen(path, "rb");
  if (NULL == fp)
  {
    rtLogError("%s : cannot open '%s'", __FUNCTION__, path);
    return RT_ERROR;
  }

  rapidjson::Document doc;
  rapidjson::ParseResult result;
  char readBuffer[FILE_BUFFER_SIZE];
  memset(readBuffer, 0, sizeof(readBuffer));
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  result = doc.ParseStream(is);

  fclose(fp);

  if (!result)
  {
    rapidjson::ParseErrorCode e = doc.GetParseError();
    rtLogError("%s : [JSON parse error : %s (%ld)]", __FUNCTION__, rapidjson::GetParseError_En(e), result.Offset());
    return RT_ERROR;
  }

  return jsonValue2rtValue(doc, val);
}

rtError rtValue2jsonFile(const rtValue& val, const char* path)
{
  if (val.getType() != RT_objectType)
  {
    rtLogError("%s : value type is not supported", __FUNCTION__);
    return RT_ERROR_NOT_IMPLEMENTED;
  }

  rapidjson::Document doc;
  doc.SetObject();

  rtObjectRef obj = val.toObject();
  rtValue allKeys;
  obj->Get("allKeys", &allKeys);
  rtArrayObject* arr = (rtArrayObject*) allKeys.toObject().getPtr();
  for (uint32_t i = 0; i < arr->length(); ++i)
  {
    rtString k = arr->get<rtString>(i);
    rtValue v = obj.get<rtValue>(k);

    rapidjson::Value j;
    rtError e = rtValue2jsonValue(v, doc, j);
    if (e != RT_OK)
      return e;

    rapidjson::Value key;
    key.SetString(k.cString(), k.byteLength(), doc.GetAllocator());
    doc.AddMember(key, j, doc.GetAllocator());
  }

  FILE* fp = fopen(path, "wb");
  if (NULL == fp)
  {
    rtLogError("%s : cannot open '%s'", __FUNCTION__, path);
    return RT_ERROR;
  }

  char writeBuffer[FILE_BUFFER_SIZE];
  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
  doc.Accept(writer);
  fclose(fp);

  return RT_OK;
}

rtError json2rtObject(const char* json, rtObjectRef& obj)
{
  rtError e;

  rtValue v;
  e = json2rtValue(json, v);
  if (e == RT_OK)
  {
    rtObjectRef o;
    e = v.getObject(o);
    if (e == RT_OK)
      obj = o;
  }

  return e;
}
