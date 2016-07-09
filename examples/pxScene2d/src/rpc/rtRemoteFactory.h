#include "rtRemoteIResolver.h"
#include "rtRemoteTypes.h"

enum rtResolverType
{
  RT_RESOLVER_MULTICAST,
  RT_RESOLVER_FILE,
  RT_RESOLVER_UNICAST
};

class rtRemoteFactory
{
private:
  rtRemoteFactory();
  ~rtRemoteFactory();
public:
  static rtRemoteIResolver* rtRemoteCreateResolver(rtRemoteEnvironment* env);
};
