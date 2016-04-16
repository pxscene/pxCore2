#include "rtRpcConfig.h"
#include <rtLog.h>

#include <assert.h>
#include <errno.h>
#include <rtLog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <limits>
#include <vector>

static std::string trim(char const* begin, char const* end)
{
  assert(begin != nullptr);
  assert(end != nullptr);
  
  while (begin && (begin < end) && isspace(*begin))
    ++begin;

  while (end && (end > begin) && isspace(*end))
    --end;

  if (begin == end) return std::string();
  return std::string(begin, (end - begin));
}

static long int to_long(char const* s)
{
  if (s == nullptr)
  {
    rtLogError("can't conver nullptr to long int");
    assert(false);
  }

  long int l = strtol(s, nullptr, 10);
  if (l == std::numeric_limits<long>::min()) // underflow
  {
    rtLogError("uint16_t underflow: %s", s);
    assert(false);
  }

  if (l == std::numeric_limits<long>::max()) // overflow
  {
    rtLogError("uint16_t overflow: %s", s);
    assert(false);
  }

  return l;
}

template<class T>
static T numeric_cast(char const* s)
{
  long int l = to_long(s);
  long int max = std::numeric_limits<T>::max();
  assert(l <= max);
  return static_cast<T>(l);
}

static std::shared_ptr<rtRpcConfig> gConf;

struct Setting
{
  char const* name;
  char const* value;
};

static Setting kDefaultSettings[] =
{
#ifdef __APPLE__
  { "rt.rpc.resolver.multicast_interface", "en0" },
#else
  { "rt.rpc.resolver.multicast_interface", "eth0" },
#endif
  { "rt.rpc.resolver.multicast_address", "224.10.0.12" },
  { "rt.rpc.resolver.multicast_address6", "ff05:0:0:0:0:0:0:201" },
  { "rt.rpc.resolver.multicast_port", "10004" },
  { "rt.rpc.default.request_timeout", "1000" },
  { "rt.rpc.resolver.locate_timeout", "1000" },
  { nullptr, nullptr }
};

std::shared_ptr<rtRpcConfig>
rtRpcConfig::getInstance()
{
  if (gConf)
    return gConf;

  gConf.reset(new rtRpcConfig());
  for (int i = 0; kDefaultSettings[i].name; ++i)
  {
    gConf->m_map.insert(std::map<std::string, std::string>::value_type(
      kDefaultSettings[i].name,
      kDefaultSettings[i].value));
  }

  std::vector<std::string> configFiles;
  char* file = getenv("RT_RPC_CONFIG");
  if (file)
    configFiles.push_back(file);

  configFiles.push_back("./rtrpc.conf");
  configFiles.push_back("/etc/rtrpc.conf");

  for (std::string const& fileName : configFiles)
  {
    // overrite any existing defaults from file
    std::shared_ptr<rtRpcConfig> confFromFile = rtRpcConfig::fromFile(fileName.c_str());
    if (confFromFile)
    {
      rtLogInfo("loading configuration settings from: %s", fileName.c_str());
      for (auto const& itr : confFromFile->m_map)
	gConf->m_map[itr.first] = itr.second;
      break;
    }
  }

  return gConf;
}

uint16_t
rtRpcConfig::getUInt16(char const* key)
{
  return numeric_cast<uint16_t>(getString(key));
}

uint32_t
rtRpcConfig::getUInt32(char const* key)
{
  return numeric_cast<uint32_t>(getString(key));
}

char const*
rtRpcConfig::getString(char const* key)
{
  if (key == nullptr)
  {
    rtLogError("can't find null key");
    assert(false);
    return nullptr;
  }

  auto itr = m_map.find(key);
  if (itr == m_map.end())
  {
    rtLogWarn("failed to find key: %s", key);
    return nullptr;
  }

  // char const* val = itr->second.c_str();
  return itr->second.c_str();
}



std::shared_ptr<rtRpcConfig>
rtRpcConfig::fromFile(char const* file)
{
  std::shared_ptr<rtRpcConfig> conf;

  if (file == nullptr)
  {
    rtLogError("null file path");
    return conf;
  }
  
  FILE* f = fopen(file, "r");
  if (!f)
  {
    rtLogDebug("can't open: %s. %s", file, strerror(errno));
    return conf;
  }

  conf.reset(new rtRpcConfig());

  std::vector<char> buff;
  buff.reserve(1024);
  buff.resize(1024);

  int line = 1;

  char* p = nullptr;
  while ((p = fgets(&buff[0], static_cast<int>(buff.size()), f)) != nullptr)
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

    assert(end && *end == '=');
    
    std::string name = trim(begin, end);

    begin = end + 1;
    end = (p + (n - 1));

    std::string val = trim(begin, end);

    rtLogDebug("LINE:(%04d) '%s'", line++, p);
    rtLogDebug("'%s' == '%s", name.c_str(), val.c_str());
  }

  fclose(f);
  return RT_OK;
}
