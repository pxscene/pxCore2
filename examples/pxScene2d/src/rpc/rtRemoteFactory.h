#include "rtRemoteIResolver.h"
#include "rtRemoteTypes.h"

enum resolver_t { MULTICAST_RESOLVER, FILE_RESOLVER, NS_RESOLVER };

class rtRemoteFactory
{
private:
  rtRemoteFactory();
  ~rtRemoteFactory();
public:
  static rtRemoteIResolver* rtRemoteCreateResolver(resolver_t type, rtRemoteEnvPtr env);
  static rtRemoteIResolver* rtRemoteCreateResolver(rtRemoteEnvPtr env);
};