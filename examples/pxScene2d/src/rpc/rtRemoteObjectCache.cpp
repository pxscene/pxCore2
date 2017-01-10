#include "rtRemoteObjectCache.h"
#include "rtRemoteConfig.h"
#include "rtRemoteEnvironment.h"

#include <map>
#include <mutex>
#include <iostream>

namespace
{
  struct Entry
  {
    rtObjectRef     Object;
    rtFunctionRef   Function;
    time_t          LastUsed;
    int             MaxIdleTime;
    bool            ShouldBeRemoved;

    bool isActive(time_t now) const
    {
      return (now - LastUsed) < MaxIdleTime;
    }

  };

  using refmap = std::map< std::string, Entry >;

  std::mutex    sMutex;
  refmap        sRefMap;
}

rtObjectRef
rtRemoteObjectCache::findObject(std::string const& id)
{
  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  return (itr != sRefMap.end()) ? itr->second.Object : rtObjectRef();
}

rtFunctionRef
rtRemoteObjectCache::findFunction(std::string const& id)
{
  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  return (itr != sRefMap.end()) ? itr->second.Function : rtFunctionRef();
}

rtError
rtRemoteObjectCache::markForRemoval(std::string const& id)
{
  rtError e = RT_OK;

  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  if (itr != sRefMap.end())
  {
    itr->second.ShouldBeRemoved = true;
    e = RT_OK;
  }
  else
  {
    e = RT_ERROR_OBJECT_NOT_FOUND;
  }

  return e;
}


rtError
rtRemoteObjectCache::insert(std::string const& id, rtFunctionRef const& ref)
{
  rtError e = RT_OK;

  Entry entry;
  entry.LastUsed = time(nullptr);
  entry.Function = ref;
  entry.MaxIdleTime = m_env->Config->cache_max_object_lifetime();
  entry.ShouldBeRemoved = false;

  std::unique_lock<std::mutex> lock(sMutex);
  auto res = sRefMap.insert(refmap::value_type(id, entry));
  if (!res.second) // entry already exists
    e = RT_ERROR_DUPLICATE_ENTRY;

  return e;
}

rtError
rtRemoteObjectCache::insert(std::string const& id, rtObjectRef const& ref)
{
  rtError e = RT_OK;

  Entry entry;
  entry.LastUsed = time(nullptr);
  entry.Object = ref;
  entry.MaxIdleTime = m_env->Config->cache_max_object_lifetime();
  entry.ShouldBeRemoved = false;

  std::unique_lock<std::mutex> lock(sMutex);
  auto res = sRefMap.insert(refmap::value_type(id, entry));
  if (!res.second) // entry already exists
    e = RT_ERROR_DUPLICATE_ENTRY;

  return e;
}

rtError
rtRemoteObjectCache::touch(std::string const& id, time_t now)
{
  rtError e = RT_OK;

  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  if (itr != sRefMap.end())
  {
    itr->second.LastUsed = now;
    e = RT_OK;
  }
  else
  {
    e = RT_ERROR_OBJECT_NOT_FOUND;
  }

  return e;
}

rtError
rtRemoteObjectCache::clear()
{
  rtLogInfo("clearing object cache");

  std::unique_lock<std::mutex> lock(sMutex);
  sRefMap.clear();

  return RT_OK;
}

rtError
rtRemoteObjectCache::erase(std::string const& id)
{
  rtError e = RT_OK;

  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  if (itr != sRefMap.end())
  {
    sRefMap.erase(itr);
    e = RT_OK;
  }
  else
  {
    e = RT_ERROR_OBJECT_NOT_FOUND;
  }

  return e;
}

rtError
rtRemoteObjectCache::removeUnused()
{
  time_t now = time(nullptr);

  std::unique_lock<std::mutex> lock(sMutex);
  for (auto itr = sRefMap.begin(); itr != sRefMap.end();)
  {
    // rtLogInfo("IsActive:%s LastUsed:%ld MaxIdleTime:%d ShouldBeRemoved:%d", itr->first.c_str(),
    //  itr->second.LastUsed, itr->second.MaxIdleTime, itr->second.ShouldBeRemoved);
    #if 0
    if (itr->second.ShouldBeRemoved && itr->second.isActive(now))
    {
      rtLogInfo("not removing:%s, should remove, but it's active",
        itr->first.c_str());
    }
    #endif

    if (!itr->second.isActive(now) && itr->second.ShouldBeRemoved)
      itr = sRefMap.erase(itr);
    else
      ++itr;
  }

  return RT_OK;
}
