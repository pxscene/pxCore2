#ifndef __RT_TYPES_H__
#define __RT_TYPES_H__

#include <map>
#include <memory>
#include <vector>

#include <rtError.h>
#include <rapidjson/document.h>

#define kInvalidSocket (-1)

class rtRemoteClient;

using rtRemoteMessage     = rapidjson::Document;
using rtRemoteMessagePtr  = std::shared_ptr<rtRemoteMessage>;
using rtSocketBuffer      = std::vector<char>;
using rtJsonDocPtr        = std::shared_ptr< rapidjson::Document >;
using rtCorrelationKey    = uint32_t;
using MessageHandler      = rtError (*)(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& msg, void* argp);

#endif
