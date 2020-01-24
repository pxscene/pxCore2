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

#include "rtPathUtils.h"
#include "rtJsonUtils.h"

const char* rtSettings::FILE_NAME = ".sparkSettings.json";
const char* rtSettings::FILE_ENV_NAME = "SPARK_SETTINGS";

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
  rtString settingsPath = filePath;

  if (settingsPath.isEmpty())
  {
    // runtime location, if available
    const char* env = getenv(FILE_ENV_NAME);
    if (env && rtFileExists(env))
      settingsPath = env;
  }

  if (settingsPath.isEmpty())
  {
    // default location, if available
    rtString homePath;
    if (RT_OK == rtGetHomeDirectory(homePath))
    {
      homePath.append(FILE_NAME);
      if (rtFileExists(homePath))
        settingsPath = homePath;
    }
  }

  // no file at default locations
  if (settingsPath.isEmpty())
    return RT_RESOURCE_NOT_FOUND;

  rtValue v;
  if (jsonFile2rtValue(settingsPath.cString(), v) == RT_OK)
  {
    rtObjectRef obj;
    if (v.getObject(obj) == RT_OK)
    {
      rtValue allKeys;
      obj->Get("allKeys", &allKeys);
      rtArrayObject* arr = (rtArrayObject*) allKeys.toObject().getPtr();
      for (uint32_t i = 0; i < arr->length(); ++i)
      {
        rtString key = arr->get<rtString>(i);
        setValue(key, obj.get<rtValue>(key));
      }
    }
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
          rtString keyStr(key, static_cast<uint32_t> (val - key) );
          setValue(keyStr, val+1);
          key = NULL; val = NULL;
        }
      }
      else if (key)
      {
        setValue(key, arg);
        key = NULL; val = NULL;
      }
    }
  }

  return RT_OK;
}

rtError rtSettings::save(const rtString& filePath) const
{
  rtMapObject* o = new rtMapObject;
  for (std::map<rtString, rtValue>::const_iterator it = mValues.begin(); it != mValues.end(); ++it)
  {
    o->Set(it->first, &it->second);
  }

  rtValue v = o;

  return rtValue2jsonFile(v, filePath.cString());
}

rtDefineObject(rtSettings, rtObject);
