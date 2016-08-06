#ifndef __RT_TYPES_H__
#define __RT_TYPES_H__

#include <memory>

enum class rtNetType      { NONE, IPV4, IPV6, ICMP };
enum class rtCastType     { NONE, UNICAST, MULTICAST, BROADCAST };
enum class rtConnType     { NONE, STREAM, DGRAM };

class rtRemoteIEndpoint;

using rtRemoteEndpointPtr = std::shared_ptr< rtRemoteIEndpoint >;

#endif
