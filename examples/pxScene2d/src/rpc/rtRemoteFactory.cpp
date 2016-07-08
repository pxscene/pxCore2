#include "rtRemoteFactory.h"
#include "rtRemoteIResolver.h"
#include "rtRemoteFileResolver.h"
#include "rtRemoteMulticastResolver.h"
#include "rtRemoteNsResolver.h"
#include "rtRemoteTypes.h"

rtRemoteFactory::rtRemoteFactory(){}
rtRemoteFactory::~rtRemoteFactory(){}

rtRemoteIResolver*
rtRemoteFactory::rtRemoteCreateResolver(rtRemoteEnvPtr env)
{
  return rtRemoteCreateResolver(MULTICAST_RESOLVER, env);
}

rtRemoteIResolver*
rtRemoteFactory::rtRemoteCreateResolver(resolver_t type, rtRemoteEnvPtr env) {
  switch (type)
  {
    case MULTICAST_RESOLVER : return new rtRemoteMulticastResolver(env);
    case FILE_RESOLVER      : return new rtRemoteFileResolver(env);
    case NS_RESOLVER        : return new rtRemoteNsResolver(env);
    default                 : return new rtRemoteMulticastResolver(env);
  }
}