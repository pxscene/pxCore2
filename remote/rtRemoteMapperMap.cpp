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

#include "rtRemoteMapperMap.h"
#include "rtRemoteEndpoint.h"
#include "rtRemoteEnvironment.h"
#include "rtError.h"
#include <map>
#include <string>
#include <mutex>
#include <thread>

rtRemoteMapperMap::rtRemoteMapperMap(rtRemoteEnvironment* env)
: rtRemoteIMapper(env)
{
  // empty
}

rtError
rtRemoteMapperMap::registerEndpoint(std::string const& objectId, rtRemoteEndpointPtr const& endpoint)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_hosted_objects[objectId] = endpoint;
  lock.unlock();
  return RT_OK;
}

rtError
rtRemoteMapperMap::deregisterEndpoint(std::string const& objectId)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_hosted_objects[objectId] = nullptr;
  lock.unlock();
  return RT_OK;
}

rtError
rtRemoteMapperMap::lookupEndpoint(std::string const& objectId, rtRemoteEndpointPtr& endpoint)
{
  auto itr = m_hosted_objects.end();
  std::unique_lock<std::mutex> lock(m_mutex);
  itr = m_hosted_objects.find(objectId);
  lock.unlock();

  if (itr != m_hosted_objects.end())
  {
    endpoint = itr->second;
  }
  return RT_OK;
}

bool
rtRemoteMapperMap::isRegistered(std::string const& objectId)
{
  auto itr = m_hosted_objects.end();
  std::unique_lock<std::mutex> lock(m_mutex);
  itr = m_hosted_objects.find(objectId);
  lock.unlock();

  if (itr != m_hosted_objects.end())
  {
    return true;
  }
  return false;
}
