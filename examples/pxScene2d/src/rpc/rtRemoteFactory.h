#include "rtRemoteIResolver.h"

enum resolver_t { MULTICAST_RESOLVER, FILE_RESOLVER, NS_RESOLVER };

class rtRemoteFactory
{
private:
  rtRemoteFactory();
public:
  static rtRemoteIResolver* rtRemoteCreateResolver();
  static rtRemoteIResolver* rtRemoteCreateResolver(resolver_t type);
};