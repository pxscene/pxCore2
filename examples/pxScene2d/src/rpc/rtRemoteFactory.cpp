#include "rtRemoteFactory.h"
#include "rtRemoteConfig.h"
#include "rtRemoteIResolver.h"
#include "rtRemoteFileResolver.h"
#include "rtRemoteMulticastResolver.h"
#include "rtRemoteNsResolver.h"
#include "rtRemoteTypes.h"

static rtResolverType
rtResolverTypeFromString(char const* s)
{
  rtResolverType t = RT_RESOLVER_MULTICAST;
  if (s != nullptr)
  {
    if (strcasecmp(s, "multicast") == 0)
      t = RT_RESOLVER_MULTICAST;
    else if (strcasecmp(s, "file") == 0)
      t = RT_RESOLVER_FILE;
    else if (strcasecmp(s, "unicast") == 0)
      t = RT_RESOLVER_UNICAST;
  }
  return t;
};

rtRemoteFactory::rtRemoteFactory()
{
  // empty
}

rtRemoteFactory::~rtRemoteFactory()
{
  // empty
}

rtRemoteIResolver*
rtRemoteFactory::rtRemoteCreateResolver(rtRemoteEnvironment* env)
{
  rtRemoteIResolver* resolver = nullptr;
  rtResolverType t = rtResolverTypeFromString(env->Config->getString("rt.rpc.resolver.type"));
  switch (t)
  {
    case RT_RESOLVER_MULTICAST:
      resolver = new rtRemoteMulticastResolver(env);
      break;
    case RT_RESOLVER_FILE:
      resolver = new rtRemoteFileResolver(env);
      break;
    case RT_RESOLVER_UNICAST:
      resolver = new rtRemoteNsResolver(env);
      break;
    default:
      resolver = new rtRemoteMulticastResolver(env);
      break;
  }
  return resolver;
}
