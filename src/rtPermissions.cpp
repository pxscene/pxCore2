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
#include "rtPathUtils.h"

#include <../remote/rapidjson/document.h>
#include <../remote/rapidjson/filereadstream.h>
#include <../remote/rapidjson/error/en.h>

#include <stdlib.h>
#include <algorithm>


// Bootstrap
const char* rtPermissions::DEFAULT_CONFIG_FILE = "./pxscenepermissions.conf";
const char* rtPermissions::CONFIG_ENV_NAME = "PXSCENE_PERMISSIONS_CONFIG";
const int rtPermissions::CONFIG_BUFFER_SIZE = 65536;
rtPermissions::assignMap_t rtPermissions::mAssignMap;
rtPermissions::roleMap_t rtPermissions::mRolesMap;
std::string rtPermissions::mConfigPath;


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

rtPermissions::permissionsMap_t permissionsJsonToMap(const rapidjson::Value& json)
{
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
		#ifndef WIN32
          rtLogWarn("%s : '%s' is not object", __PRETTY_FUNCTION__, sectionName);
        else
		  rtLogWarn("%s : '%s' is not object", __FUNCTION__, sectionName);
	    #endif
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
          	#ifndef WIN32
			  rtLogWarn("%s : '%s' is not array", __PRETTY_FUNCTION__, allowBlockName);
			#else
			  rtLogWarn("%s : '%s' is not array", __FUNCTION__, allowBlockName);
		    #endif
            continue;
          }
          for (rapidjson::SizeType k = 0; k < arr.Size(); k++)
          {
            const rapidjson::Value& str = arr[k];
            if (!str.IsString())
            {
		      #ifndef WIN32
                rtLogWarn("%s : '%s' item is not string", __PRETTY_FUNCTION__, allowBlockName);
              #else
			    rtLogWarn("%s : '%s' item is not string", __FUNCTION__, allowBlockName);
			  #endif
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

rtPermissions::permissionsMap_t permissionsObjectToMap(const rtObjectRef& permissionsObject)
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

  rtString currentDir;
  rtGetCurrentDirectory(currentDir);
  #ifndef WIN32
    rtLogDebug("%s : currentDir='%s'", __PRETTY_FUNCTION__, currentDir.cString());
  #else
	rtLogDebug("%s : currentDir='%s'", __FUNCTION__, currentDir.cString());
  #endif
  FILE* fp = fopen(s, "rb");
  if (NULL == fp)
  {
	#ifndef WIN32
      rtLogDebug("%s : cannot open '%s'", __PRETTY_FUNCTION__, s);
    #else
	  rtLogDebug("%s : cannot open '%s'", __FUNCTION__, s);
	#endif
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
	#ifndef WIN32
      rtLogWarn("%s : [JSON parse error : %s (%ld)]", __PRETTY_FUNCTION__, rapidjson::GetParseError_En(e), result.Offset());
    #else
      rtLogWarn("%s : [JSON parse error : %s (%ld)]", __FUNCTION__, rapidjson::GetParseError_En(e), result.Offset());
	#endif
	return RT_FAIL;
  }

  if (!doc.IsObject() || !doc.HasMember("roles") || !doc.HasMember("assign"))
  {
	#ifndef WIN32
      rtLogWarn("%s : no 'roles'/'assign' in json", __PRETTY_FUNCTION__);
    #else
	  rtLogWarn("%s : no 'roles'/'assign' in json", __FUNCTION__);
    #endif
	return RT_FAIL;
  }

  const rapidjson::Value& assign = doc["assign"];
  const rapidjson::Value& roles = doc["roles"];
  if (!assign.IsObject() || !roles.IsObject())
  {
	#ifndef WIN32
      rtLogWarn("%s : 'roles'/'assign' are not objects", __PRETTY_FUNCTION__);
	#else
	  rtLogWarn("%s : 'roles'/'assign' are not objects", __FUNCTION__);
    #endif
	return RT_FAIL;
  }

  for (rapidjson::Value::ConstMemberIterator itr = assign.MemberBegin(); itr != assign.MemberEnd(); ++itr)
  {
    const rapidjson::Value& key = itr->name;
    const rapidjson::Value& val = itr->value;
    if (!key.IsString() || !val.IsString())
    {
      #ifndef WIN32
	    rtLogWarn("%s : 'assign' key/value is not string", __PRETTY_FUNCTION__);
      #else
		rtLogWarn("%s : 'assign' key/value is not string", __FUNCTION__);
	  #endif
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
	  #ifndef WIN32	
        rtLogWarn("%s : 'roles' key/value is not string/object", __PRETTY_FUNCTION__);
	  #else
		rtLogWarn("%s : 'roles' key/value is not string/object", __FUNCTION__);
	  #endif
      continue;
    }
    mRolesMap[key.GetString()] = permissionsJsonToMap(val);
  }
  #ifndef WIN32
    rtLogInfo("%s : %ld roles, %ld assigned urls", __PRETTY_FUNCTION__, mRolesMap.size(), mAssignMap.size());
  #else
	rtLogInfo("%s : %ld roles, %ld assigned urls", __FUNCTION__, mRolesMap.size(), mAssignMap.size());
  #endif
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
		#ifndef WIN32
          rtLogDebug("%s : mapping for '%s': '%s'", __PRETTY_FUNCTION__, origin, it->second.c_str());
		#else
		   rtLogDebug("%s : mapping for '%s': '%s'", __FUNCTION__, origin, it->second.c_str());
	    #endif
        return RT_OK;
      }
    }
    #ifndef WIN32
      rtLogDebug("%s : no mapping for '%s'", __PRETTY_FUNCTION__, origin);
    #else
	  rtLogDebug("%s : no mapping for '%s'", __FUNCTION__, origin);
    #endif  
  }

  return RT_FAIL;
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

rtError rtPermissions::allows(const char* s, rtPermissions::Type type, bool& o) const
{
  o = true; // default

  if (s && !mPermissionsMap.empty())
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
    if (it != mPermissionsMap.end())
    {
      o = it->second;
    }
  }

  return (o && mParent) ? mParent->allows(s, type, o) : RT_OK;
}
