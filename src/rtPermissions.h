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

#ifndef _RT_PERMISSIONS
#define _RT_PERMISSIONS

#include "rtObject.h"

#include <map>
#include <string>

typedef std::map<std::string, bool> permissionsMap_t;

class rtPermissions
{
public:
    rtPermissions();
    ~rtPermissions();

    rtError loadBootstrapConfig(const char* filename = NULL);
    rtError clearBootstrapConfig();
    rtError setOrigin(const char* origin);
    rtError set(const rtObjectRef& permissionsObject);
    rtError setParent(const rtPermissions* parent);
    rtError allows(const char* url, bool& o) const;
    rtError findMatch(const char* url, std::string& match) const;

private:
    static const char* DEFAULT_CONFIG_FILE;
    static const char* CONFIG_ENV_NAME;

    static std::map<std::string, std::string> mAssignMap;
    static std::map<std::string, permissionsMap_t> mRolesMap;
    static std::string mConfigPath;

    permissionsMap_t mPermissionsMap;
    const rtPermissions* mParent;
};

#endif
