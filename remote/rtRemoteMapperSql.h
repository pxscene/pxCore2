#ifndef __RT_REMOTE_ENDPOINT_MAPPER_SQL_H__
#define __RT_REMOTE_ENDPOINT_MAPPER_SQL_H__

#include "rtRemoteMapper.h"
#include "rtRemoteEndPoint.h"
#include "rtError.h"
#include <string>

class rtRemoteEnvironment;

class rtRemoteMapperSql : public virtual rtRemoteIMapper
{
public:
  rtRemoteMapperSql(rtRemoteEnvironment* env);
  //~rtRemoteMapperSql();

public:
  virtual rtError registerEndpoint(std::string const& objectId, rtRemoteEndPointPtr const& endpoint) override;
  virtual rtError deregisterEndpoint(std::string const& objectId) override;
  virtual rtError lookupEndpoint(std::string const& objectId, rtRemoteEndPointPtr& endpoint) override; 
  virtual bool    isRegistered(std::string const& objectId) override;
};

#endif