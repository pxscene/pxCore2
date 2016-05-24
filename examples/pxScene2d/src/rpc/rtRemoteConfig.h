#ifndef __RT_RPC_CONFIG_H__
#define __RT_RPC_CONFIG_H__

#include <map>
#include <memory>
#include <string>

#include <assert.h>
#include <stdint.h>
#include "../rtError.h"

class rtRemoteConfig
{
public:
  static std::shared_ptr<rtRemoteConfig> getInstance(bool reloadConfiguration = false);
  static std::shared_ptr<rtRemoteConfig> fromFile(char const* file);

public:
  char const*	getString(char const* key);
  uint16_t  	getUInt16(char const* key);
  uint32_t      getUInt32(char const* key);
  int32_t	getInt32(char const* key);

private:
  std::map< std::string, std::string> m_map;
};

template<class T>
T rtRemoteSetting(char const* /*s*/)
{
  assert(false);
  return T();
}

template<>
inline char const* rtRemoteSetting(char const* s)
{
  return rtRemoteConfig::getInstance()->getString(s);
}

template<>
inline uint16_t rtRemoteSetting(char const* s)
{
  return rtRemoteConfig::getInstance()->getUInt16(s);
}

template<>
inline uint32_t rtRemoteSetting(char const* s)
{
  return rtRemoteConfig::getInstance()->getUInt32(s);
}

template<>
inline int32_t rtRemoteSetting(char const* s)
{
  return rtRemoteConfig::getInstance()->getInt32(s);
}

#endif
