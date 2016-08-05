#ifndef __RT_TYPES_H__
#define __RT_TYPES_H__

#include <map>
#include <memory>
#include <vector>

#include <rtError.h>
#include <rapidjson/document.h>

#define kInvalidSocket (-1)

class rtRemoteClient;
class rtRemoteIEndpoint;

enum class rtNetType      { NONE, IPV4, IPV6, ICMP };
enum class rtCastType     { NONE, UNICAST, MULTICAST, BROADCAST };
enum class rtConnType     { NONE, STREAM, DGRAM };

using rtRemoteMessage     = rapidjson::Document;
using rtRemoteMessagePtr  = std::shared_ptr<rtRemoteMessage>;
using rtSocketBuffer      = std::vector<char>;
using rtJsonDocPtr        = std::shared_ptr< rapidjson::Document >;
using MessageHandler      = rtError (*)(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& msg, void* argp);
using rtRemoteEndpointPtr = std::shared_ptr< rtRemoteIEndpoint >;

#endif
