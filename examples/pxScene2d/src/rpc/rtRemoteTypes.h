#ifndef __RT_TYPES_H__
#define __RT_TYPES_H__

#include <rtError.h>
#include <rtObject.h>
#include <rapidjson/document.h>

#include <memory>
#include <vector>

#define kInvalidSocket (-1)

class rtRemoteClient;
class rtRemoteServer;
class rtRemoteConfig;
class rtRemoteStreamSelector;
class rtObjectCache;

struct rtRemoteEnvironment
{
  rtRemoteEnvironment();
  ~rtRemoteEnvironment();

  void shutdown();

  rtRemoteConfig*           Config;
  rtRemoteServer*           Server;
  rtObjectCache*            ObjectCache;
  rtRemoteStreamSelector*   StreamSelector;

  uint32_t RefCount;
  bool     Initialized;
};

using rtRemoteEnvPtr = rtRemoteEnvironment*;

using rtSocketBuffer = std::vector<char>;
using rtJsonDocPtr = std::shared_ptr< rapidjson::Document >;
using rtCorrelationKey = uint32_t;
using rtRemoteMessageHandler = std::function<rtError (std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& msg)>;
using rtRemoteInactivityHandler = std::function<rtError(time_t lastMessageTime, time_t now)>;

#endif
