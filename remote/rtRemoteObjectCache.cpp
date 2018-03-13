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

#include "rtRemoteObjectCache.h"
#include "rtRemoteConfig.h"
#include "rtRemoteEnvironment.h"

#include <map>
#include <mutex>
#include <iostream>
#include <chrono>

using std::chrono::steady_clock;

namespace
{
  struct Entry
  {
    rtObjectRef              Object;
    rtFunctionRef            Function;
    steady_clock::time_point LastUsed;
    std::chrono::seconds     MaxIdleTime;
    bool                     Unevictable;

    bool isActive(const steady_clock::time_point& now) const
    {
      return (now - LastUsed) < MaxIdleTime;
    }

  };

  using refmap = std::map< std::string, Entry >;

  std::mutex    sMutex;
  refmap        sRefMap;
  size_t        sHighMark = 10000;
}

rtObjectRef
rtRemoteObjectCache::findObject(std::string const& id)
{
  rtObjectRef obj;
  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  if (itr != sRefMap.end())
    obj = itr->second.Object;
  return obj;
}

rtFunctionRef
rtRemoteObjectCache::findFunction(std::string const& id)
{
  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  return (itr != sRefMap.end()) ? itr->second.Function : rtFunctionRef();
}

rtError
rtRemoteObjectCache::markUnevictable(std::string const& id, bool state)
{
  rtError e = RT_OK;

  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  if (itr != sRefMap.end())
  {
    itr->second.Unevictable = state;
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

  if (!ref)
  {
    rtLogError("trying to insert null reference");
    return RT_ERROR_INVALID_ARG;
  }

  RT_ASSERT(!!ref);

  Entry entry;
  entry.LastUsed = std::chrono::steady_clock::now();
  entry.Function = ref;
  entry.MaxIdleTime = std::chrono::seconds(m_env->Config->cache_max_object_lifetime());
  entry.Unevictable = false;

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

  if (!ref)
  {
    rtLogError("trying to insert null reference");
    return RT_ERROR_INVALID_ARG;
  }

  RT_ASSERT(!!ref);

  Entry entry;
  entry.LastUsed = std::chrono::steady_clock::now();
  entry.Object = ref;
  entry.MaxIdleTime = std::chrono::seconds(m_env->Config->cache_max_object_lifetime());
  entry.Unevictable = false;

  std::unique_lock<std::mutex> lock(sMutex);
  auto res = sRefMap.insert(refmap::value_type(id, entry));
  if (!res.second) // entry already exists
    e = RT_ERROR_DUPLICATE_ENTRY;

  return e;
}

rtError
rtRemoteObjectCache::touch(std::string const& id, std::chrono::steady_clock::time_point now)
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
  auto now = std::chrono::steady_clock::now();

  std::unique_lock<std::mutex> lock(sMutex);
  for (auto itr = sRefMap.begin(); itr != sRefMap.end();)
  {
    // rtLogInfo("IsActive:%s LifeTime:%lld MaxIdleTime:%lld Unevictable:%d", itr->first.c_str(),
    //           std::chrono::duration_cast<std::chrono::seconds>(now - itr->second.LastUsed).count(),
    //           itr->second.MaxIdleTime.count(), itr->second.Unevictable);
    #if 0
    if (!itr->second.Unevictable && itr->second.isActive(now))
    {
      rtLogInfo("not removing:%s, should remove, but it's active",
        itr->first.c_str());
    }
    #endif

    if (!itr->second.Unevictable && !itr->second.isActive(now))
      itr = sRefMap.erase(itr);
    else
      ++itr;
  }

  if (sRefMap.size() > sHighMark)
  {
    rtLogWarn("Cache reached high mark, current size=%zu", sRefMap.size());
  }

  return RT_OK;
}
