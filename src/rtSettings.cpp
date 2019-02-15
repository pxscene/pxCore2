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

#include "rtSettings.h"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <string.h>

const int rtSettings::FILE_BUFFER_SIZE = 65536;

rtSettings::rtSettings()
{
}

rtSettings::~rtSettings()
{
}

rtSettingsRef rtSettings::instance()
{
  static rtSettingsRef instance = new rtSettings;
  return instance;
}

rtError rtSettings::value(const rtString& key, rtValue& value) const
{
  std::map<rtString, rtValue>::const_iterator it = mValues.find(key);
  if (it != mValues.end())
  {
    value = it->second;
    return RT_OK;
  }
  return RT_ERROR;
}

rtError rtSettings::setValue(const rtString& key, const rtValue& value)
{
  mValues[key] = value;
  return RT_OK;
}

rtError rtSettings::keys(std::vector<rtString>& keys) const
{
  keys.clear();
  for (std::map<rtString, rtValue>::const_iterator it = mValues.begin(); it != mValues.end(); ++it)
    keys.push_back(it->first);
  return RT_OK;
}

rtError rtSettings::remove(const rtString& key)
{
  std::map<rtString, rtValue>::iterator it = mValues.find(key);
  if (it != mValues.end())
  {
    mValues.erase(it);
    return RT_OK;
  }
  return RT_ERROR;
}

rtError rtSettings::clear()
{
  mValues.clear();
  return RT_OK;
}

rtError rtSettings::loadFromFile(const rtString& filePath)
{
  FILE* fp = fopen(filePath.cString(), "rb");
  if (NULL == fp)
  {
    rtLogError("%s : cannot open '%s'", __FUNCTION__, filePath.cString());
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

  if (!doc.IsObject())
  {
    rtLogError("%s : no root object in json", __FUNCTION__);
    return RT_ERROR;
  }

  for (rapidjson::Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
  {
    const rapidjson::Value& key = itr->name;
    if (!key.IsString())
    {
      rtLogWarn("%s : key is not string", __FUNCTION__);
      continue;
    }

    const rapidjson::Value& val = itr->value;
    rtValue value;
    if (val.IsNull())
      value = rtValue();
    else if (val.IsBool())
      value = val.GetBool();
    else if (val.IsString())
      value = val.GetString();
    else if (val.IsInt())
      value = val.GetInt();
    else if (val.IsUint())
      value = val.GetUint();
    else if (val.IsInt64())
      value = val.GetInt64();
    else if (val.IsUint64())
      value = val.GetUint64();
    // All float values are convertible to double, no float in rapidjson
    else if (val.IsDouble())
      value = val.GetDouble();
    else
    {
      rtLogWarn("%s : bad value type: %d for key %s", __FUNCTION__, val.GetType(), key.GetString());
      continue;
    }

    setValue(key.GetString(), value);
  }

  return RT_OK;
}

rtError rtSettings::loadFromArgs(int argc, char* argv[])
{
  const char* key = NULL, *val = NULL;
  for (int i=1;i<argc;i++)
  {
    const char* arg = argv[i];
    if (arg)
    {
      if (arg[0] == '-')
      {
        key = isalpha(arg[1]) ? arg+1 : NULL;
        val = strchr(arg, '=');
        if (key && val)
        {
          rtString keyStr(key, val-key);
          setValue(keyStr, val+1);
          key = NULL, val = NULL;
        }
      }
      else if (key)
      {
        setValue(key, arg);
        key = NULL, val = NULL;
      }
    }
  }

  return RT_OK;
}

rtError rtSettings::save(const rtString& filePath) const
{
  FILE* fp = fopen(filePath.cString(), "wb");
  if (NULL == fp)
  {
    rtLogError("%s : cannot open '%s'", __FUNCTION__, filePath.cString());
    return RT_ERROR;
  }

  rapidjson::Document doc;
  doc.SetObject();

  for (std::map<rtString, rtValue>::const_iterator it = mValues.begin(); it != mValues.end(); ++it)
  {
    rapidjson::Value val;
    if (it->second.isEmpty())
      val.SetNull();
    else
    {
      rtType t = it->second.getType();
      if (t == RT_boolType)
        val = it->second.toBool();
      else if (t == RT_int8_tType)
        val = it->second.toInt8();
      else if (t == RT_uint8_tType)
        val = it->second.toUInt8();
      else if (t == RT_intType || t == RT_int32_tType)
        val = it->second.toInt32();
      else if (t == RT_uint32_tType)
        val = it->second.toUInt32();
      else if (t == RT_int64_tType)
        val = it->second.toInt64();
      else if (t == RT_uint64_tType)
        val = it->second.toUInt64();
      else if (t == RT_floatType)
        val = it->second.toFloat();
      else if (t == RT_doubleType)
        val = it->second.toDouble();
      else if (t == RT_stringType || t == RT_rtStringType)
      {
        rtString s = it->second.toString();
        val.SetString(s.cString(), s.byteLength(), doc.GetAllocator());
      }
      else
      {
        rtLogWarn("%s : bad value type: %c for key %s", __FUNCTION__, t, it->first.cString());
        continue;
      }
    }
    rapidjson::Value key;
    key.SetString(it->first.cString(), it->first.byteLength(), doc.GetAllocator());
    doc.AddMember(key, val, doc.GetAllocator());
  }

  char writeBuffer[FILE_BUFFER_SIZE];
  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
  doc.Accept(writer);
  fclose(fp);

  return RT_OK;
}

rtDefineObject(rtSettings, rtObject);
