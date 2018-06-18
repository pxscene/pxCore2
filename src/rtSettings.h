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

#ifndef _RT_SETTINGS
#define _RT_SETTINGS

#include "rtObject.h"
#include "rtRef.h"

#include <map>

class rtSettings;
typedef rtRef<rtSettings> rtSettingsRef;

class rtSettings : public rtObject
{
public:
  ~rtSettings();

  rtDeclareObject(rtSettings, rtObject);

  static rtSettingsRef instance();

  // if key is found then value is set and RT_OK is returned. otherwise RT_ERROR is returned
  rtError value(const rtString& key, rtValue& value) const;

  rtError setValue(const rtString& key, const rtValue& value);
  rtError keys(std::vector<rtString>& keys) const;

  // removes the key (if present) and its corresponding value. returns RT_OK on success and RT_ERROR if key is not found
  rtError remove(const rtString& key);

  rtError clear();

  // loads the settings from a file and returns RT_OK on success or RT_ERROR on failure
  rtError loadFromFile(const rtString& filePath);

  // there are two ways settings can specified via the command line
  //   Method 1: -settingName <value>
  //   Method 2: -settingName=value
  rtError loadFromArgs(int argc, char* argv[]);

  // saves the settings (in json format) to disk with the specified file name
  // returns RT_OK on success or RT_ERROR on failure
  rtError save(const rtString& filePath) const;

private:
  rtSettings();

  static const int FILE_BUFFER_SIZE;

  std::map<rtString, rtValue> mValues;

  friend class rtSettingsTest;
};

#endif
