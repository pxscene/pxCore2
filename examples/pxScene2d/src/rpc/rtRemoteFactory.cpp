#include "rtRemoteFactory.h"
#include "rtRemoteIResolver.h"
#include "rtRemoteFileResolver.h"
#include "rtRemoteMulticastResolver.h"

rtRemoteFactory::rtRemoteFactory(){}

rtRemoteIResolver*
rtRemoteFactory::rtRemoteCreateResolver()
{
    return rtRemoteCreateResolver(MULTICAST_RESOLVER);
}

rtRemoteIResolver*
rtRemoteFactory::rtRemoteCreateResolver(resolver_t type) {
  switch (type)
  {
    case MULTICAST_RESOLVER : return new rtRemoteMulticastResolver();
    case FILE_RESOLVER      : return new rtRemoteFileResolver();
    default                 : return new rtRemoteMulticastResolver();
  }
}