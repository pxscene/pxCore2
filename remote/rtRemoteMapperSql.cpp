#include "rtRemoteMapperSql.h"
#include "rtRemoteEndPoint.h"
#include "rtRemoteEnvironment.h"
#include "rtError.h"
#include <string>

rtRemoteMapperSql::rtRemoteMapperSql(rtRemoteEnvironment* env)
: rtRemoteIMapper(env)
{
  // TODO
}

rtError
rtRemoteMapperSql::registerEndpoint(std::string const& /*objectId*/, rtRemoteEndPointPtr const& /*endpoint*/)
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
rtRemoteMapperSql::lookupEndpoint(std::string const& /*objectId*/, rtRemoteEndPointPtr& /*endpoint*/)
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