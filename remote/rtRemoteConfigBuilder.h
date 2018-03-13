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

#ifndef __RT_REMOTE_CONFIG_BUILDER_H__
#define __RT_REMOTE_CONFIG_BUILDER_H__

#include <stdint.h>
#include <string>
#include <map>

class rtRemoteConfigBuilder
{
public:
  rtRemoteConfigBuilder();

  static rtRemoteConfigBuilder* getDefaultConfig();
  static rtRemoteConfigBuilder* fromFile(char const* file);

  rtRemoteConfig* build() const;

  char const* getString(char const* key) const;
  uint16_t    getUInt16(char const* key) const;
  uint32_t    getUInt32(char const* key) const;
  int32_t     getInt32(char const* key) const;
  bool        getBool(char const* key) const;

private:
  std::map< std::string, std::string > m_map;
};

#endif
