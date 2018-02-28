#ifndef __RT_REMOTE_OBJECT_RESOLVER_H__
#define __RT_REMOTE_OBJECT_RESOLVER_H__

#include <memory>
#include <string>
#include <rtError.h>
#include <stdint.h>

#include "rtRemoteSocketUtils.h"

#include "rtRemoteTypes.h"

class rtRemoteIResolver
{
public:
  virtual ~rtRemoteIResolver() { }
  virtual rtError open(sockaddr_storage const& rpc_endpoint) = 0;
  virtual rtError close() = 0;
  virtual rtError registerObject(std::string const& name, sockaddr_storage const& endpoint) = 0;
  virtual rtError locateObject(std::string const& name, sockaddr_storage& endpoint, uint32_t timeout) = 0;
  virtual rtError unregisterObject(std::string const& name) = 0;
};

#endif
