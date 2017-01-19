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