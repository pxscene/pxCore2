/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "rtRemoteFactory.h"
#include "rtRemoteConfig.h"
#include "rtRemoteIResolver.h"
#include "rtRemoteMulticastResolver.h"
#include "rtRemoteTypes.h"
#include "rtRemoteEnvironment.h"

static rtResolverType
rtResolverTypeFromString(std::string const& resolverType)
{
  char const* s = resolverType.c_str();

  rtResolverType t = RT_RESOLVER_MULTICAST;
  if (s != nullptr)
  {
    if (strcasecmp(s, "multicast") == 0)
      t = RT_RESOLVER_MULTICAST;
    else if (strcasecmp(s, "file") == 0)
      t = RT_RESOLVER_FILE;
    else if (strcasecmp(s, "unicast") == 0)
      t = RT_RESOLVER_UNICAST;
    else
      RT_ASSERT(false);
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
  rtResolverType t = rtResolverTypeFromString(env->Config->resolver_type());
  switch (t)
  {
    case RT_RESOLVER_MULTICAST:
      resolver = new rtRemoteMulticastResolver(env);
      break;
    default:
      assert(false);
      break;
  }
  return resolver;
}
