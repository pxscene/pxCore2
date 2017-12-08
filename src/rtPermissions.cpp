/*

 pxCore Copyright 2005-2017 John Robinson

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

#include <../remote/rapidjson/document.h>
#include <../remote/rapidjson/filereadstream.h>
#include <../remote/rapidjson/error/en.h>

#include <stdlib.h>

const char* rtPermissions::DEFAULT_CONFIG_FILE = "./pxscenepermissions.conf";
const char* rtPermissions::CONFIG_ENV_NAME = "PXSCENE_PERMISSIONS_CONFIG";
std::map<std::string, std::string> rtPermissions::mAssignMap;
std::map<std::string, permissionsMap_t> rtPermissions::mRolesMap;
std::string rtPermissions::mConfigPath;

template<typename Map> typename Map::const_iterator
findBestWildcardMatch(Map const& map, typename Map::key_type const& key)
{
  if (key.empty())
    return map.end();

  size_t bestMatchLength = 0;
  typename Map::const_iterator it = map.begin();
  typename Map::const_iterator best = map.end();
  for (; it != map.end(); ++it)
  {
    const char* url = key.c_str();
    const char* wildcard = it->first.c_str();
    size_t len = 0;
    const char* wildcardAlt = NULL;
    size_t lenAlt = 0;
    for (; *url; url++)
    {
      while (*wildcard == '*')
      {
        wildcardAlt = ++wildcard;
        lenAlt = len;
      }
      if (*url == *wildcard)
      {
        wildcard++;
        len++;
      }
      else if (wildcardAlt != NULL)
      {
        wildcard = wildcardAlt;
        len = lenAlt;
      }
      else
      {
        break;
      }
    }
    for (; *wildcard == '*'; wildcard++);
    if (*wildcard == 0 && len >= bestMatchLength)
    {
      bestMatchLength = len;
      best = it;
    }
  }

  return best;
}

permissionsMap_t permissionsJsonToMap(const rapidjson::Value& json)
{
  permissionsMap_t ret;
  for (int i = 0; i < 3; i++)
  {
    // first level... "url", "serviceManager", "features"
    const char* sectionName = i == 1 ? "serviceManager" : (i == 2 ? "features" : "url");
    if (json.HasMember(sectionName))
    {
      const rapidjson::Value& sec = json[sectionName];
      for (int j = 0; j < 2; j++)
      {
        // second level... "allow", "block"
        const char* allowBlockName = j == 0 ? "allow": "block";
        if (sec.HasMember(allowBlockName))
        {
          const rapidjson::Value& arr = sec[allowBlockName];
          for (rapidjson::SizeType k = 0; k < arr.Size(); k++)
          {
            // third level... array of urls
            std::string key = i == 1 ? "serviceManager://" : (i == 2 ? "feature://" : "");
            key += arr[k].GetString();
            ret[key] = j == 0;
          }
        }
      }
    }
  }
  return ret;
}

permissionsMap_t permissionsObjectToMap(const rtObjectRef& permissionsObject)
{
  permissionsMap_t ret;
  for (int i = 0; i < 3; i++)
  {
    // first level... "url", "serviceManager", "features"
    const char* sectionName = i == 1 ? "serviceManager" : (i == 2 ? "features" : "url");
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
            std::string key = i == 1 ? "serviceManager://" : (i == 2 ? "feature://" : "");
            key += arr.get<rtString>(k).cString();
            ret[key] = j == 0;
          }
        }
      }
    }
  }
  return ret;
}

rtPermissions::rtPermissions()
  : mParent(NULL)
{
  loadBootstrapConfig();
}

rtPermissions::~rtPermissions()
{
}

rtError rtPermissions::loadBootstrapConfig(const char* filename)
{
  char const* s = filename;
  if (!s)
  {
    s = getenv(CONFIG_ENV_NAME);
  }
  if (!s)
  {
    s = DEFAULT_CONFIG_FILE;
  }
  if (mConfigPath == s)
  {
    // already did try this path
    return RT_OK;
  }

  // try load... first clean up previous config
  clearBootstrapConfig();
  mConfigPath = s;

  FILE* fp = fopen(s, "rb");
  if (NULL == fp)
  {
    rtLogError("Permissions config read error : cannot open '%s'", s);
    return RT_FAIL;
  }

  rapidjson::Document doc;
  rapidjson::ParseResult result;
  char readBuffer[65536];
  memset(readBuffer, 0, sizeof(readBuffer));
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  result = doc.ParseStream(is);
  fclose(fp);

  if (!result)
  {
    rapidjson::ParseErrorCode e = doc.GetParseError();
    rtLogInfo("Permissions config read error : [JSON parse error : %s (%ld)]",rapidjson::GetParseError_En(e), result.Offset());
    return RT_FAIL;
  }

  if (!doc.HasMember("roles") || !doc.HasMember("assign"))
  {
    rtLogInfo("Permissions config invalid");
    return RT_FAIL;
  }

  const rapidjson::Value& assign = doc["assign"];
  for (rapidjson::Value::ConstMemberIterator itr = assign.MemberBegin(); itr != assign.MemberEnd(); ++itr)
  {
    mAssignMap[itr->name.GetString()] = itr->value.GetString();
  }
  const rapidjson::Value& roles = doc["roles"];
  for (rapidjson::Value::ConstMemberIterator itr = roles.MemberBegin(); itr != roles.MemberEnd(); ++itr)
  {
    mRolesMap[itr->name.GetString()] = permissionsJsonToMap(itr->value);
  }

  return RT_OK;
}

rtError rtPermissions::clearBootstrapConfig()
{
  mAssignMap.clear();
  mRolesMap.clear();
  mConfigPath.clear();

  return RT_OK;
}

rtError rtPermissions::setOrigin(const char* origin)
{
  if (origin && *origin && !mAssignMap.empty())
  {
    std::map<std::string, std::string>::const_iterator it =
      findBestWildcardMatch(mAssignMap, origin);

    if (it != mAssignMap.end())
    {
      std::map<std::string, permissionsMap_t>::const_iterator jt =
        mRolesMap.find(it->second);

      if (jt != mRolesMap.end())
      {
        mPermissionsMap = jt->second;
      }
    }
  }

  return RT_OK;
}

rtError rtPermissions::set(const rtObjectRef& permissionsObject)
{
  mPermissionsMap = permissionsObjectToMap(permissionsObject);

  return RT_OK;
}

rtError rtPermissions::setParent(const rtPermissions* parent)
{
  mParent = parent;

  return RT_OK;
}

rtError rtPermissions::allows(const char* url, bool& o) const
{
  bool allow = true; // default

  if (url && *url && !mPermissionsMap.empty())
  {
    rtString urlOrigin = rtUrlGetOrigin(url);
    if (!urlOrigin.isEmpty())
    {
      permissionsMap_t::const_iterator it =
        findBestWildcardMatch(mPermissionsMap, urlOrigin.cString());

      if (it != mPermissionsMap.end())
      {
        allow = it->second;
        if (allow && mParent)
        {
          return mParent->allows(url, o);
        }
      }
    }
  }

  o = allow;
  return RT_OK;
}

rtError rtPermissions::findMatch(const char* url, std::string& match) const
{
  if (url && *url && !mPermissionsMap.empty())
  {
    permissionsMap_t::const_iterator it =
      findBestWildcardMatch(mPermissionsMap, url);

    if (it != mPermissionsMap.end())
    {
      match = it->first;
      return RT_OK;
    }
  }

  return RT_FAIL;
}
