#ifndef __RT_REMOTE_ENDPOINT_MAPPER_MAP_H__
#define __RT_REMOTE_ENDPOINT_MAPPER_MAP_H__

#include "rtRemoteMapper.h"
#include "rtRemoteEndpoint.h"
#include "rtError.h"
#include <string>
#include <mutex>
#include <map>

class rtRemoteEnvironment;

class rtRemoteMapperMap : public virtual rtRemoteIMapper
{
public:
  rtRemoteMapperMap(rtRemoteEnvironment* env);

public:
  virtual rtError registerEndpoint(std::string const& objectId, rtRemoteEndpointPtr const& endpoint) override;
  virtual rtError deregisterEndpoint(std::string const& objectId) override;
  virtual rtError lookupEndpoint(std::string const& objectId, rtRemoteEndpointPtr& endpoint) override; 
  virtual bool    isRegistered(std::string const& objectId) override;

private:
  using RegisteredObjectsMap = std::map< std::string, rtRemoteEndpointPtr >;

private: 
  RegisteredObjectsMap m_hosted_objects;
  std::mutex           m_mutex;
};

#endif