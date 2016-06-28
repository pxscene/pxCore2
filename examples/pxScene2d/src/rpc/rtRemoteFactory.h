#include "rtRemoteResolver.h"

enum resolver_t { MULTICAST_RESOLVER, FILE_RESOLVER };

class rtRemoteFactory
{
private:
  rtRemoteFactory();
public:
  static rtRemoteIResolver* rtRemoteCreateResolver();
  static rtRemoteIResolver* rtRemoteCreateResolver(resolver_t type);
};