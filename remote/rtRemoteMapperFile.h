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
