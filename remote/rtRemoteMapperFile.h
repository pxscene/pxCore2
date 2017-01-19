#ifndef __RT_REMOTE_ENDPOINT_MAPPER_FILE_H__
#define __RT_REMOTE_ENDPOINT_MAPPER_FILE_H__

#include "rtRemoteMapper.h"
#include "rtRemoteEndpoint.h"
#include "rtError.h"
#include <string>
#include <mutex>

class rtRemoteEnvironment;

class rtRemoteMapperFile : public virtual rtRemoteIMapper
{
public:
  rtRemoteMapperFile(rtRemoteEnvironment* env);
  ~rtRemoteMapperFile();

public:
  virtual rtError registerEndpoint(std::string const& objectId, rtRemoteEndpointPtr const& endpoint) override;
  virtual rtError deregisterEndpoint(std::string const& objectId) override;
  virtual rtError lookupEndpoint(std::string const& objectId, rtRemoteEndpointPtr& endpoint) override; 
  virtual bool    isRegistered(std::string const& objectId) override;

private:
  std::string m_file_path;
  std::mutex  m_mutex;
};

#endif