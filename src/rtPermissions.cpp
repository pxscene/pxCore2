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

#include "rtPermissions.h"

#include "rtUrlUtils.h"
#include "rtPathUtils.h"

#include "../remote/rapidjson/document.h"
#include "../remote/rapidjson/filereadstream.h"
#include "../remote/rapidjson/error/en.h"

#include <stdlib.h>
#include <fstream>

const char* rtPermissions::DEFAULT_CONFIG_FILE = "./sparkpermissions.conf";
const char* rtPermissions::CONFIG_ENV_NAME = "SPARK_PERMISSIONS_CONFIG";
const char* rtPermissions::ENABLED_ENV_NAME = "SPARK_PERMISSIONS_ENABLED";
bool rtPermissions::mEnabled = false;
rtObjectRef rtPermissions::mConfig = NULL;

rtPermissions::rtPermissions(const char* origin)
  : mOrigin(rtUrlGetOrigin(origin))
  , mParent(NULL)
{
  static bool didInit = false;
  if (!didInit)
  {
    init();
    didInit = true;
  }

  if (!mConfig && mEnabled)
    rtLogWarn("no permissions config");

  rtString role;
  if (mConfig)
  {
    rtObjectRef assign = mConfig.get<rtObjectRef>("assign");
    if (!mOrigin.isEmpty() && assign)
    {
      rtString s;
      if (find(assign, mOrigin.cString(), s) == RT_OK)
      {
        role = assign.get<rtString>(s.cString());
        rtLogInfo("permissions role '%s' for origin '%s", role.cString(), mOrigin.cString());
      }
    }
  }

  if (!mOrigin.isEmpty() && role.isEmpty())
    rtLogWarn("no permissions role for origin '%s'", mOrigin.cString());

  if (!role.isEmpty())
  {
    rtObjectRef roles = mConfig.get<rtObjectRef>("roles");
    if (roles)
      mRole = roles.get<rtObjectRef>(role.cString());
  }

  if (!role.isEmpty() && !mRole)
    rtLogWarn("no config for permissions role '%s'", role.cString());
}

rtPermissions::~rtPermissions()
{
}

rtDefineObject(rtPermissions, rtObject);
rtDefineMethod(rtPermissions, allows);

rtError rtPermissions::init(const char* filename)
{
  rtError e = RT_OK;

  const char* s = getenv(ENABLED_ENV_NAME);
  if (s != NULL)
  {
    rtString envVal(s);
    mEnabled = 0 == envVal.compare("true") || 0 == envVal.compare("1");
  }
  else
  {
    mEnabled = true;
  }

  rtLogInfo("permissions %s", mEnabled ? "enabled" : "disabled");

  if (mEnabled)
  {
    if (!filename)
      filename = getenv(CONFIG_ENV_NAME);
    if (!filename)
      filename = DEFAULT_CONFIG_FILE;

    rtString s;
    e = file2str(filename, s);
    if (e == RT_OK)
    {
      rtObjectRef obj;
      const char* sStr = s.cString();
      e = json2obj(sStr, obj);
      if (e == RT_OK)
      {
        rtLogInfo("using permissions config '%s", filename);
        mConfig = obj;
      }
    }
  }

  if (e != RT_OK)
    rtLogError("cannot read permissions config '%s'", filename);

  return e;
}

rtError rtPermissions::set(const char* json)
{
  rtError e = RT_OK;

  rtObjectRef obj;
  e = json2obj(json, obj);
  if (e == RT_OK)
    mRole = obj;
  else
    rtLogError("cannot set permissions json");

  return e;
}

rtError rtPermissions::set(const rtObjectRef& obj)
{
  mRole = obj;
  return RT_OK;
}

rtError rtPermissions::setParent(const rtPermissionsRef& parent)
{
  mParent = parent;
  return RT_OK;
}

rtError rtPermissions::allows(const char* s, rtPermissions::Type type) const
{
  if (!mEnabled)
    return RT_OK;

  if (s == NULL || *s == 0)
    return RT_OK; // allow empty

  const char* t = type2str(type);
  if (mRole == NULL)
  {
    if (mOrigin.isEmpty())
      return RT_OK; // allow from file system
    return RT_ERROR_NOT_ALLOWED;
  }  

  rtObjectRef o = mRole.get<rtObjectRef>(t);
  if (o == NULL)
  {
    rtLogDebug("no type %s in permissions role", t);
    return RT_ERROR_NOT_ALLOWED;
  }

  rtString str = s;
  if (type == DEFAULT)
  {
    // need only origin part of URL
    rtString origin = rtUrlGetOrigin(s);
    if (!origin.isEmpty())
      str = origin;
  }

  rtString allowFound;
  rtObjectRef allow = o.get<rtObjectRef>("allow");
  if (allow != NULL)
    find(allow, str.cString(), allowFound);

  rtString blockFound;
  rtObjectRef block = o.get<rtObjectRef>("block");
  if (block != NULL)
    find(block, str.cString(), blockFound);

  rtLogDebug("found '%s' (allow) and '%s' (block) for '%s' of type %s", allowFound.cString(), blockFound.cString(), s, t);
  if ((blockFound.isEmpty() && allowFound.isEmpty()) || blockFound.byteLength() > allowFound.byteLength())
    return RT_ERROR_NOT_ALLOWED;

  if (!mParent)
    return RT_OK;
  return mParent->allows(s, type);
}

rtError rtPermissions::allows(const rtString& url, bool& o) const
{
  rtError e = allows(url.cString(), rtPermissions::DEFAULT);
  if (RT_OK == e)
    o = true;
  else if (RT_ERROR_NOT_ALLOWED == e)
    o = false;
  else
    return RT_FAIL;

  return RT_OK;
}

rtError rtPermissions::file2str(const char* file, rtString& s)
{
  rtError e = RT_OK;

  if (rtFileExists(file))
  {
    std::ifstream is(file, std::ifstream::binary);
    if (is)
    {
      is.seekg(0, is.end);
      int length = is.tellg();
      is.seekg(0, is.beg);
      if (length > 0)
      {
        char * buffer = new char[length];
        is.read(buffer, length);
        if (!is)
          e = RT_FAIL;
        else
          s = rtString(buffer, length);
        is.close();
        delete[] buffer;
      }
      else
      {
        s = rtString();
      }
    }
    else
    {
      rtLogError("error opening '%s'", file);
      e = RT_FAIL;
    }
  }

  return e;
}

namespace
{
  rtError jsonValue2rtValue(const rapidjson::Value& jsonValue, rtValue& v)
  {
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
    else if (jsonValue.IsString())
    {
      v = jsonValue.GetString();
    }
    else
    {
      rtLogError("%s : value is not string/array/object", __FUNCTION__);
      return RT_FAIL;
    }
    return RT_OK;
  }
}

rtError rtPermissions::json2obj(const char* json, rtObjectRef& obj)
{
  if (!json || *json == 0)
  {
    rtLogError("%s : empty", __FUNCTION__);
    return RT_FAIL;
  }

  rapidjson::Document doc;
  rapidjson::ParseResult result = doc.Parse(json);

  if (!result)
  {
    rapidjson::ParseErrorCode e = doc.GetParseError();
    rtLogError("%s : [JSON parse error : %s (%ld)]", __FUNCTION__, rapidjson::GetParseError_En(e), result.Offset());
    return RT_FAIL;
  }

  if (!doc.IsObject())
  {
    rtLogError("%s : no root object", __FUNCTION__);
    return RT_FAIL;
  }

  rtError e;
  rtValue val2;
  e = jsonValue2rtValue(doc, val2);
  if (e == RT_OK)
    obj = val2.toObject();

  return e;
}

rtError rtPermissions::find(const rtObjectRef& obj, const char* s, rtString& found)
{
  rtValue length;
  rtObjectRef arr;
  if (obj->Get("length", &length) == RT_OK)
  {
    arr = obj;
  }
  else
  {
    rtValue allKeys;
    if (obj->Get("allKeys", &allKeys) == RT_OK)
    {
      arr = allKeys.toObject();
      arr->Get("length", &length);
    }
  }

  if (length.isEmpty())
  {
    rtLogError("permissions list is not an array/map");
    return RT_FAIL;
  }

  bool hasMatches = false;
  rtString best;
  size_t bestMatchLength = 0;
  const int n = length.toInt32();
  for (int i = 0; i < n; ++i)
  {
    rtValue item;
    if (arr->Get(i, &item) != RT_OK)
      continue;

    rtString itemStr = item.toString();
    const char* url = s;
    const char* w = itemStr.cString();
    const char* wAlt = NULL;
    size_t len = 0;
    size_t lenAlt = 0;
    for (; *url && w; url++)
    {
      for (; *w == '*'; wAlt = ++w, lenAlt = len);
      bool equal = *url == *w;
      w = equal ? w + 1 : wAlt;
      len = equal ? len + 1 : lenAlt;
    }
    if (w)
    {
      for (; *w == '*'; w++);
      if (*w == 0 && len >= bestMatchLength)
      {
        bestMatchLength = len;
        best = itemStr;
        hasMatches = true;
      }
    }
  }

  if (hasMatches)
  {
    found = best;
    return RT_OK;
  }
  return RT_PROP_NOT_FOUND;
}

const char* rtPermissions::type2str(Type t)
{
  switch (t)
  {
    case DEFAULT: return "url";
    case SERVICE: return "serviceManager";
    case FEATURE: return "features";
    case WAYLAND: return "applications";
    default: return NULL;
  }
}
