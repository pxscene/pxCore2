#ifndef __RT_REMOTE_ENDPOINT_MAPPER_H__
#define __RT_REMOTE_ENDPOINT_MAPPER_H__

#include "rtRemoteEndPoint.h"
#include "rtError.h"
#include <string>

class rtRemoteEnvironment;

class rtRemoteIMapper
{
public:
  virtual ~rtRemoteIMapper() { }

public:
  virtual rtError registerEndpoint(std::string const& objectId, rtRemoteEndPointPtr const& endpoint) = 0;
  virtual rtError deregisterEndpoint(std::string const& objectId) = 0;
  virtual rtError lookupEndpoint(std::string const& objectId, rtRemoteEndPointPtr& endpoint) = 0; 
  virtual bool    isRegistered(std::string const& objectId) = 0;

protected:
  rtRemoteIMapper(rtRemoteEnvironment* env)
  : m_env(env)
  { }

protected:
  rtRemoteEnvironment* m_env; 
};

#endif