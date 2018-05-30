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

#include "rtRemoteMapperSql.h"
#include "rtRemoteEndpoint.h"
#include "rtRemoteEnvironment.h"
#include "rtError.h"
#include <string>

rtRemoteMapperSql::rtRemoteMapperSql(rtRemoteEnvironment* env)
: rtRemoteIMapper(env)
{
  // TODO
}

rtError
rtRemoteMapperSql::registerEndpoint(std::string const& /*objectId*/, rtRemoteEndpointPtr const& /*endpoint*/)
{
  // TODO
  return RT_FAIL;
}

rtError
rtRemoteMapperSql::deregisterEndpoint(std::string const& /*objectId*/)
{
  // TODO
  return RT_FAIL;
}

rtError
rtRemoteMapperSql::lookupEndpoint(std::string const& /*objectId*/, rtRemoteEndpointPtr& /*endpoint*/)
{
  // TODO
  return RT_FAIL;
}

bool
rtRemoteMapperSql::isRegistered(std::string const& /*objectId*/)
{
  // TODO
  return false;
}
