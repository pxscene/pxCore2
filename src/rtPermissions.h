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

class rtPermissions;
typedef rtRef<rtPermissions> rtPermissionsRef;

class rtPermissions : public rtObject
{
public:
  rtPermissions(const char* origin = NULL);
  virtual ~rtPermissions();

  rtDeclareObject(rtPermissions, rtObject);
  rtMethod1ArgAndReturn("allows", allows, rtString, bool);

  enum Type
  {
    DEFAULT = 0,
    SERVICE,
    FEATURE,
    WAYLAND,
    TYPE_COUNT
  };

  static rtError init(const char* filename = NULL);

  rtError set(const char* json);
  rtError set(const rtObjectRef& obj);
  rtError setParent(const rtPermissionsRef& parent);
  rtError allows(const char* s, rtPermissions::Type type) const;
  rtError allows(const rtString& url, bool& o) const;

  // Parsing
  static rtError file2str(const char* file, rtString& s);
  static rtError json2obj(const char* json, rtObjectRef& obj);
  static rtError find(const rtObjectRef& obj, const char* s, rtString& found); // obj = map or array
  static const char* type2str(Type t);

protected:
  // Bootstrap
  static const char* DEFAULT_CONFIG_FILE;
  static const char* ENABLED_ENV_NAME;
  static const char* CONFIG_ENV_NAME;
  static bool mEnabled;
  static rtObjectRef mConfig;

  rtString mOrigin;
  rtPermissionsRef mParent;
  rtObjectRef mRole;
};

#endif
