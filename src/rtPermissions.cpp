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
#include "rtJsonUtils.h"

const char* rtPermissions::DEFAULT_CONFIG_FILE = "./sparkpermissions.conf";
const char* rtPermissions::CONFIG_ENV_NAME = "SPARK_PERMISSIONS_CONFIG";
const char* rtPermissions::ENABLED_ENV_NAME = "SPARK_PERMISSIONS_ENABLED";
const char* rtPermissions::LOCAL_FILE_ORIGIN = "file://";

bool rtPermissions::mEnabled = false;
rtObjectRef rtPermissions::mConfig = NULL;

rtPermissions::rtPermissions(const char* origin)
  : mOrigin(rtUrlGetOrigin(origin))
  , mParent(NULL)
{
  if (mOrigin.isEmpty())
    mOrigin = LOCAL_FILE_ORIGIN;

  //
  // read env. variables and config
  static bool didInit = false;
  if (!didInit)
  {
    init();
    didInit = true;
  }
  if (!mConfig && mEnabled)
    rtLogWarn("no permissions config");

  //
  // find role in conf
  rtString role;
  if (mConfig)
  {
    rtObjectRef assign = mConfig.get<rtObjectRef>("assign");
    if (assign)
    {
      rtString s;
      if (find(assign, mOrigin.cString(), s) == RT_OK)
        role = assign.get<rtString>(s.cString());
    }
  }
  if (!role.isEmpty())
  {
    rtObjectRef roles = mConfig.get<rtObjectRef>("roles");
    if (roles)
      set(roles.get<rtObjectRef>(role.cString()));
  }

  //
  // logging
  if (role.isEmpty())
    rtLogWarn("no permissions role for '%s'", mOrigin.cString());
  else if (!mRole)
    rtLogWarn("no config for permissions role '%s'", role.cString());
  else
    rtLogDebug("permissions role '%s' for '%s'", role.cString(), mOrigin.cString());
}

rtPermissions::~rtPermissions()
{
}

rtDefineObject(rtPermissions, rtObject);
rtDefineMethod(rtPermissions, allows);
rtDefineProperty(rtPermissions, storageQuota);

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

    rtValue v;
    e = jsonFile2rtValue(filename, v);
    if (e == RT_OK)
    {
      rtObjectRef obj;
      e = v.getObject(obj);
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
  rtError e;

  rtValue v;
  e = json2rtValue(json, v);
  if (e == RT_OK)
  {
    rtObjectRef obj;
    e = v.getObject(obj);
    if (e == RT_OK)
      mRole = obj;
  }

  if (e != RT_OK)
    rtLogError("cannot set permissions json");

  return e;
}

rtError rtPermissions::set(const rtObjectRef& obj)
{
  mRole = obj;

  uint32_t quota = 0;
  if (mRole)
  {
    rtObjectRef o = mRole.get<rtObjectRef>(type2str(STORAGE));
    if (o)
      quota = o.get<uint32_t>("allow");
  }
  mStorageQuota = quota;

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
    if (mOrigin == LOCAL_FILE_ORIGIN)
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

rtError rtPermissions::getStorageQuota(uint32_t& o) const
{
  o = mStorageQuota;
  return RT_OK;
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
    case RTREMOTE: return "rtRemote";
    case STORAGE: return "storage";
    default: return NULL;
  }
}
