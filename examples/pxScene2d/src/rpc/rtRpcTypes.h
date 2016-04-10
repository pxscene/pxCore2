#ifndef __RT_TYPES_H__
#define __RT_TYPES_H__

#include <rtError.h>
#include <rapidjson/document.h>

#include <memory>
#include <string>

class rtRpcClient;

typedef std::shared_ptr< rapidjson::Document > rtJsonDocPtr_t;
typedef uint32_t rtCorrelationKey_t;

using rtRpcMessageHandler_t = std::function<rtError (std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr_t const& msg)>;
using rtRpcInactivityHandler_t = std::function<rtError(time_t lastMessageTime, time_t now)>;

#define kDefaultRequestTimeout 2000

std::string rtStrError(int e);

#endif
