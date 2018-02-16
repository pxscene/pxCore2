#include "rtRemoteConfig.h"
#include "rtRemoteConfigBuilder.h"
#include "rtRemoteSocketUtils.h"

#include <rtError.h>
#include <rtLog.h>
#include <errno.h>
#include <rtLog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

static bool myisspace(int c) 
{
   return c == ' ';
}

static inline std::string& ltrim(std::string& s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
    std::not1(std::ptr_fun<int, bool>(myisspace))));
  return s;
}

static inline std::string& rtrim(std::string& s)
{
  s.erase(std::find_if(s.rbegin(), s.rend(),
    std::not1(std::ptr_fun<int, bool>(myisspace))).base(), s.end());
  return s;
}

static inline std::string& trim(std::string& s)
{
  return ltrim(rtrim(s));
}

static std::string trim(char const* begin, char const* end)
{
  if (begin == nullptr)
  {
    rtLogError("invalid 'begin' pointer");
    return std::string();
  }

  if (end == nullptr)
  {
    rtLogError("invalid 'end' pointer");
    return std::string();
  }

  std::string s(begin, (end - begin));
  return trim(s);
}

template<class T>
static T numeric_cast(char const* s, std::function<T (const char *nptr, char **endptr, int base)> converter)
{
  if (s == nullptr)
  {
    // TODO: this should throw an exception and/or return something clearly
    // invalid
    rtLogError("can't conver nullptr to long int");
    return T();
  }

  T const val = converter(s, nullptr, 10);
  if (val == std::numeric_limits<T>::min()) // underflow
  {
    rtLogError("underflow: %s", s);
    // TODO
    return T();
  }

  if (val == std::numeric_limits<T>::max()) // overflow
  {
    rtLogError("overflow: %s", s);
    // TODO
    return T();
  }

  return val;
}

struct Setting
{
  char const* name;
  char const* value;
};

rtRemoteConfigBuilder*
rtRemoteConfigBuilder::getDefaultConfig()
{
  // defaults are loaded in constructor
  rtRemoteConfigBuilder* conf = new rtRemoteConfigBuilder();

  std::vector<std::string> configFiles;
  char* file = getenv("RT_RPC_CONFIG");
  if (file)
    configFiles.push_back(file);

  configFiles.push_back("./rtremote.conf");
  configFiles.push_back("/etc/rtremote.conf");
  configFiles.push_back("./rtrpc.conf");
  configFiles.push_back("/etc/rtrpc.conf");

  for (std::string const& fileName : configFiles)
  {
    // overrite any existing defaults from file
    std::unique_ptr<rtRemoteConfigBuilder> confFromFile(rtRemoteConfigBuilder::fromFile(fileName.c_str()));
    if (confFromFile)
    {
      rtLogInfo("loading configuration settings (%d) from: %s",
        static_cast<int>(confFromFile->m_map.size()), fileName.c_str());
      for (auto const& itr : confFromFile->m_map)
      {
        rtLogDebug("'%s' -> '%s'", itr.first.c_str(), itr.second.c_str());
        conf->m_map[itr.first] = itr.second;
      }
    }
    else
    {
      rtLogDebug("failed to find settings file: %s", fileName.c_str());
    }
  }

  return conf;
}

bool
rtRemoteConfigBuilder::getBool(char const* key) const
{
  bool b = false;
  char const* val = getString(key);
  if (val != nullptr)
    b = strcasecmp(val, "true") == 0;
  return b;
}

uint16_t
rtRemoteConfigBuilder::getUInt16(char const* key) const
{
  return numeric_cast<uint16_t>(getString(key), strtoul);
}

int32_t
rtRemoteConfigBuilder::getInt32(char const* key) const
{
  return numeric_cast<int32_t>(getString(key), strtol);
}

uint32_t
rtRemoteConfigBuilder::getUInt32(char const* key) const
{
  return numeric_cast<uint32_t>(getString(key), strtoul);
}

char const*
rtRemoteConfigBuilder::getString(char const* key) const
{
  RT_ASSERT(key != nullptr);
  RT_ASSERT(strlen(key) > 0);

  if (key == nullptr)
    return nullptr;

  if (strlen(key) == 0)
    return nullptr;

  auto itr = m_map.find(key);
  if (itr == m_map.end())
  {
    rtLogWarn("failed to find key: %s", key);
    return nullptr;
  }

  return itr->second.c_str();
}



rtRemoteConfigBuilder*
rtRemoteConfigBuilder::fromFile(char const* file)
{
  if (file == nullptr)
  {
    rtLogError("null file path");
    return nullptr;
  }

  std::unique_ptr<FILE, int (*)(FILE *)> f(fopen(file, "r"), fclose);
  if (!f)
  {
    rtLogDebug("can't open: %s. %s", file, strerror(errno));
    return nullptr;
  }

  rtRemoteConfigBuilder* builder(new rtRemoteConfigBuilder());

  std::vector<char> buff;
  buff.reserve(1024);
  buff.resize(1024);

  int line = 1;

  char* p = nullptr;
  while ((p = fgets(&buff[0], static_cast<int>(buff.size()), f.get())) != nullptr)
  {
    if (!p) break;

    size_t n = strlen(p);
    if (n == 0)
      continue;

    if (p[n-1] == '\n')
      p[n-1] = '\0';

    char const* begin = p;
    char const* end = p + 1;

    while (end && (*end != '='))
      end++;

    if (end && *end != '=')
    {
      rtLogWarn("invalid configuration line: '%s'", p);
      continue;
    }
    
    std::string name = trim(begin, end);

    begin = end + 1;
    end = (p + (n-1));

    std::string val = trim(begin, end);

    rtLogDebug("LINE:(%04d) '%s'", line++, p);
    rtLogDebug("'%s' == '%s'", name.c_str(), val.c_str());

    auto itr = builder->m_map.find(name);
    if (itr != builder->m_map.end())
    {
      if (itr->second != val)
      {
        rtLogDebug("overwriting configuration val '%s' -> '%s' with '%s'",
            itr->first.c_str(), itr->second.c_str(), val.c_str());
      }
    }
    builder->m_map[name] = val;

  }

  return builder;
}
