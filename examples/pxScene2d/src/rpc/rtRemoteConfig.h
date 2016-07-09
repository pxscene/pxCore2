#ifndef __RT_RPC_CONFIG_H__
#define __RT_RPC_CONFIG_H__

#include <map>
#include <memory>
#include <string>

#include <assert.h>
#include <stdint.h>
#include <rtError.h>

class rtRemoteEnvironment;

class rtRemoteConfig
{
  // must use rtRemoteEnvrionment to fetch configuration
  friend class rtRemoteEnvironment;

private:
  static rtRemoteConfig* getInstance();
  static rtRemoteConfig* fromFile(char const* file);

public:
  char const* getString(char const* key);
  uint16_t    getUInt16(char const* key);
  uint32_t    getUInt32(char const* key);
  int32_t     getInt32(char const* key);
  bool        getBool(char const* key);

private:
  std::map< std::string, std::string> m_map;
};

#endif
