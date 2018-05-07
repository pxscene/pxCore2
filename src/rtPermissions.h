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

#ifndef _RT_PERMISSIONS
#define _RT_PERMISSIONS

#include "rtObject.h"
#include "rtRef.h"

#include <map>
#include <string>
#include <utility>

class rtPermissions;
typedef rtRef<rtPermissions> rtPermissionsRef;

class rtPermissions : public rtObject
{
public:
  rtPermissions(const char* origin = NULL, const char* filepath = NULL);
  virtual ~rtPermissions();

  rtDeclareObject(rtPermissions, rtObject);
  rtMethod1ArgAndReturn("allows", allows, rtString, bool);

  enum Type
  {
    DEFAULT = 0,
    SERVICE,
    FEATURE,
    WAYLAND
  };

  rtError set(const rtObjectRef& permissionsObject);
  rtError setParent(const rtPermissionsRef& parent);
  rtError allows(const char* s, rtPermissions::Type type) const;
  rtError allows(const rtString& url, bool& o) const;

protected:
  // Wildcard stuff
  typedef std::pair<std::string, Type> wildcard_t;
  typedef std::map<wildcard_t, bool> permissionsMap_t;
  typedef std::map<wildcard_t, std::string> assignMap_t;
  typedef std::map<std::string, permissionsMap_t> roleMap_t;
  // Extends std::map::find by supporting wildcard_t as map keys.
  // Key with the highest length w/o wildcards (*) is preferred
  template<typename Map> typename Map::const_iterator
  static findWildcard(Map const& map, typename Map::key_type const& key);

  // Parsing
  static permissionsMap_t permissionsJsonToMap(const void* jsonValue);
  static permissionsMap_t permissionsObjectToMap(const rtObjectRef& permissionsObject);

  // Bootstrap
  static const char* DEFAULT_CONFIG_FILE;
  static const int CONFIG_BUFFER_SIZE;
  static const char* ENABLED_ENV_NAME;
  static const char* CONFIG_ENV_NAME;
  static rtError loadConfig(const char* filepath);
  static assignMap_t mAssignMap;
  static roleMap_t mRolesMap;
  static std::string mConfigPath;
  static bool mEnabled;

  permissionsMap_t mPermissionsMap;
  rtPermissionsRef mParent;

  friend class rtPermissionsTest;
};

#endif
