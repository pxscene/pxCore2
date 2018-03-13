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

#include <map>
#include <string>
#include <utility>

class rtPermissions
{
public:
    rtPermissions();
    ~rtPermissions();

    enum Type
    {
      DEFAULT = 0,
      SERVICE,
      FEATURE,
      WAYLAND
    };

    // Bootstrap
    static rtError loadBootstrapConfig(const char* filename = NULL);
    static rtError clearBootstrapConfig();

    rtError setOrigin(const char* origin);
    rtError set(const rtObjectRef& permissionsObject);
    rtError setParent(const rtPermissions* parent);
    rtError allows(const char* s, rtPermissions::Type type, bool& o) const;
    bool allows(const char* s, rtPermissions::Type type) const { bool a; allows(s, type, a); return a; }

    // Wildcard stuff
    typedef std::pair<std::string, Type> wildcard_t;
    typedef std::map<wildcard_t, bool> permissionsMap_t;
    typedef std::map<wildcard_t, std::string> assignMap_t;
    typedef std::map<std::string, permissionsMap_t> roleMap_t;

    // Extends std::map::find by supporting wildcard_t as map keys.
    // Key with the highest length w/o wildcards (*) is preferred
    template<typename Map> typename Map::const_iterator
    static findWildcard(Map const& map, typename Map::key_type const& key);

private:
    // Bootstrap
    static const char* DEFAULT_CONFIG_FILE;
    static const char* CONFIG_ENV_NAME;
    static const int CONFIG_BUFFER_SIZE;
    static assignMap_t mAssignMap;
    static roleMap_t mRolesMap;
    static std::string mConfigPath;

    permissionsMap_t mPermissionsMap;
    const rtPermissions* mParent;
};

#endif
