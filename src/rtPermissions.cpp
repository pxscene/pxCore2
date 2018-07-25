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
#include <algorithm>


// Bootstrap
const char* rtPermissions::DEFAULT_CONFIG_FILE = "./sparkpermissions.conf";
const char* rtPermissions::CONFIG_ENV_NAME = "SPARK_PERMISSIONS_CONFIG";
const char* rtPermissions::ENABLED_ENV_NAME = "SPARK_PERMISSIONS_ENABLED";
const int rtPermissions::CONFIG_BUFFER_SIZE = 65536;
rtPermissions::assignMap_t rtPermissions::mAssignMap;
rtPermissions::roleMap_t rtPermissions::mRolesMap;
std::string rtPermissions::mConfigPath;
bool rtPermissions::mEnabled;


template<typename Map> typename Map::const_iterator
rtPermissions::findWildcard(Map const& map, typename Map::key_type const& key)
{
  size_t bestMatchLength = 0;
  typename Map::const_iterator best = map.end();
  for (typename Map::const_iterator it = map.begin(); it != map.end(); ++it)
  {
    if (it->first.second != key.second)
    {
      continue;
    }
    const char* url = key.first.c_str();
    const char* w = it->first.first.c_str();
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
        best = it;
      }
    }
  }
  if (best == map.end() && key.second != DEFAULT)
  {
    typename Map::key_type key2 = key;
    key2.second = DEFAULT;
    return findWildcard(map, key2);
  }
  return best;
}

rtPermissions::permissionsMap_t rtPermissions::permissionsJsonToMap(const void* jsonValue)
{
  const rapidjson::Value& json = *static_cast<const rapidjson::Value*>(jsonValue);
  rtPermissions::permissionsMap_t ret;
  for (int i = 0; i < 4; i++)
  {
    // first level... "url", "serviceManager", "features"
    const char* sectionName = i == 1 ? "serviceManager" : (i == 2 ? "features" : (i == 3 ? "applications" : "url"));
    if (json.HasMember(sectionName))
    {
      const rapidjson::Value& sec = json[sectionName];
      if (!sec.IsObject())
      {
        rtLogWarn("%s : '%s' is not object", __FUNCTION__, sectionName);
        continue;
      }
      for (int j = 0; j < 2; j++)
      {
        // second level... "allow", "block"
        const char* allowBlockName = j == 0 ? "allow": "block";
        if (sec.HasMember(allowBlockName))
        {
          const rapidjson::Value& arr = sec[allowBlockName];
          if (!arr.IsArray())
          {
            rtLogWarn("%s : '%s' is not array", __FUNCTION__, allowBlockName);
            continue;
          }
          for (rapidjson::SizeType k = 0; k < arr.Size(); k++)
          {
            const rapidjson::Value& str = arr[k];
            if (!str.IsString())
            {
              rtLogWarn("%s : '%s' item is not string", __FUNCTION__, allowBlockName);
              continue;
            }
            // third level... array of urls
            rtPermissions::wildcard_t w;
            w.first = arr[k].GetString();
            w.second = rtPermissions::Type(i);
            ret[w] = j == 0;
          }
        }
      }
    }
  }
  return ret;
}

rtPermissions::permissionsMap_t rtPermissions::permissionsObjectToMap(const rtObjectRef& permissionsObject)
{
  rtPermissions::permissionsMap_t ret;
  for (int i = 0; i < 4; i++)
  {
    // first level... "url", "serviceManager", "features"
    const char* sectionName = i == 1 ? "serviceManager" : (i == 2 ? "features" : (i == 3 ? "applications" : "url"));
    const rtObjectRef& sec = permissionsObject.get<rtObjectRef>(sectionName);
    if (sec)
    {
      for (int j = 0; j < 2; j++)
      {
        // second level... "allow", "block"
        const char* allowBlockName = j == 0 ? "allow": "block";
        const rtObjectRef& arr = sec.get<rtObjectRef>(allowBlockName);
        if (arr)
        {
          uint32_t len = arr.get<uint32_t>("length");
          for (uint32_t k = 0; k < len; k++)
          {
            // third level... array of urls
            rtPermissions::wildcard_t w;
            w.first = arr.get<rtString>(k).cString();
            w.second = rtPermissions::Type(i);
            ret[w] = j == 0;
          }
        }
      }
    }
  }
  return ret;
}

rtPermissions::rtPermissions(const char* origin, const char* filepath)
  : mParent(NULL)
{
  static bool didCheck = false;
  if (!didCheck)
  {
    didCheck = true;
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
  }

  if (mEnabled)
  {
    loadConfig(filepath);

    if (origin && *origin && !mAssignMap.empty())
    {
      wildcard_t w;
      w.first = origin;
      w.second = DEFAULT;
      assignMap_t::const_iterator it = findWildcard(mAssignMap, w);
      if (it != mAssignMap.end())
      {
        roleMap_t::const_iterator jt = mRolesMap.find(it->second);
        if (jt != mRolesMap.end())
        {
          mPermissionsMap = jt->second;
          rtLogDebug("%s : permissions for '%s': '%s'", __FUNCTION__, origin, it->second.c_str());
        }
      }
    }
  }
}

rtPermissions::~rtPermissions()
{
}

rtError rtPermissions::loadConfig(const char* filepath)
{
  if (!filepath)
    filepath = getenv(CONFIG_ENV_NAME);

  if (!filepath)
    filepath = DEFAULT_CONFIG_FILE;

  // do not reload
  if (mConfigPath == filepath)
    return RT_OK;

  // try load... first clean up previous config
  mAssignMap.clear();
  mRolesMap.clear();
  mConfigPath.clear();
  mConfigPath = filepath;

  rtString currentDir;
  rtGetCurrentDirectory(currentDir);
  rtLogDebug("%s : currentDir='%s'", __FUNCTION__, currentDir.cString());

  FILE* fp = fopen(filepath, "rb");
  if (NULL == fp)
  {
    rtLogWarn("%s : cannot open '%s'", __FUNCTION__, filepath);
    return RT_FAIL;
  }

  rapidjson::Document doc;
  rapidjson::ParseResult result;
  char readBuffer[CONFIG_BUFFER_SIZE];
  memset(readBuffer, 0, sizeof(readBuffer));
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  result = doc.ParseStream(is);
  fclose(fp);

  if (!result)
  {
    rapidjson::ParseErrorCode e = doc.GetParseError();
    rtLogWarn("%s : [JSON parse error : %s (%ld)]", __FUNCTION__, rapidjson::GetParseError_En(e), result.Offset());
    return RT_FAIL;
  }

  if (!doc.IsObject() || !doc.HasMember("roles") || !doc.HasMember("assign"))
  {
    rtLogWarn("%s : no 'roles'/'assign' in json", __FUNCTION__);
    return RT_FAIL;
  }

  const rapidjson::Value& assign = doc["assign"];
  const rapidjson::Value& roles = doc["roles"];
  if (!assign.IsObject() || !roles.IsObject())
  {
    rtLogWarn("%s : 'roles'/'assign' are not objects", __FUNCTION__);
    return RT_FAIL;
  }

  for (rapidjson::Value::ConstMemberIterator itr = assign.MemberBegin(); itr != assign.MemberEnd(); ++itr)
  {
    const rapidjson::Value& key = itr->name;
    const rapidjson::Value& val = itr->value;
    if (!key.IsString() || !val.IsString())
    {
      rtLogWarn("%s : 'assign' key/value is not string", __FUNCTION__);
      continue;
    }
    wildcard_t w;
    w.first = key.GetString();
    w.second = DEFAULT;
    mAssignMap[w] = val.GetString();
  }

  for (rapidjson::Value::ConstMemberIterator itr = roles.MemberBegin(); itr != roles.MemberEnd(); ++itr)
  {
    const rapidjson::Value& key = itr->name;
    const rapidjson::Value& val = itr->value;
    if (!key.IsString() || !val.IsObject())
    {
      rtLogWarn("%s : 'roles' key/value is not string/object", __FUNCTION__);
      continue;
    }
    mRolesMap[key.GetString()] = permissionsJsonToMap(&val);
  }

  rtLogInfo("%s : %ld roles, %ld assigned urls", __FUNCTION__, mRolesMap.size(), mAssignMap.size());
  return RT_OK;
}

rtError rtPermissions::set(const rtObjectRef& permissionsObject)
{
  mPermissionsMap = permissionsObjectToMap(permissionsObject);

  return RT_OK;
}

rtError rtPermissions::setParent(const rtPermissionsRef& parent)
{
  mParent = parent;

  return RT_OK;
}

rtError rtPermissions::allows(const char* s, rtPermissions::Type type) const
{
  if (s && mEnabled && !mPermissionsMap.empty())
  {
    wildcard_t w;
    w.second = type;
    if (type == DEFAULT)
    {
      w.first = rtUrlGetOrigin(s).cString();
    }
    if (type != DEFAULT || w.first.empty())
    {
      w.first = s;
    }
    permissionsMap_t::const_iterator it = findWildcard(mPermissionsMap, w);
    if (it != mPermissionsMap.end() && false == it->second)
    {
      return RT_ERROR_NOT_ALLOWED;
    }
  }

  return mParent ? mParent->allows(s, type) : RT_OK;
}

rtError rtPermissions::allows(const rtString& url, bool& o) const
{
  rtError e = allows(url.cString(), rtPermissions::DEFAULT);
  o = RT_OK == e;
  return RT_OK;
}

rtDefineObject(rtPermissions, rtObject);
rtDefineMethod(rtPermissions, allows);
