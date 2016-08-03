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
