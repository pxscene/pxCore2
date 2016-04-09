#ifndef __RT_TYPES_H__
#define __RT_TYPES_H__

#include <rapidjson/document.h>
#include <memory>

typedef std::shared_ptr< rapidjson::Document > rtJsonDocPtr_t;
typedef uint32_t rtCorrelationKey_t;

#define kDefaultRequestTimeout 2000

std::string rtStrError(int e);

#endif
