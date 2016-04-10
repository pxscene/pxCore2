#include "rtObjectCache.h"


#include <map>
#include <mutex>

namespace
{
  struct Entry
  {
    rtObjectRef   	Object;
    rtFunctionRef 	Function;
    time_t		LastUsed;
  };

  using refmap = std::map< std::string, Entry >;

  std::mutex 		sMutex;
  refmap 		sRefMap;
}

rtObjectRef
rtObjectCache::findObject(std::string const& id)
{
  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  return (itr != sRefMap.end()) ? itr->second.Object : rtObjectRef();
}

rtFunctionRef
rtObjectCache::findFunction(std::string const& id)
{
  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  return (itr != sRefMap.end()) ? itr->second.Function : rtFunctionRef();
}

rtError
rtObjectCache::insert(std::string const& id, rtFunctionRef const& ref)
{
  Entry e;
  e.LastUsed = time(nullptr);
  e.Function = ref;

  std::unique_lock<std::mutex> lock(sMutex);
  auto res = sRefMap.insert(refmap::value_type(id, e));
  return res.second ? RT_OK : RT_FAIL;
}

rtError
rtObjectCache::insert(std::string const& id, rtObjectRef const& ref)
{
  Entry e;
  e.LastUsed = time(nullptr);
  e.Object = ref;

  std::unique_lock<std::mutex> lock(sMutex);
  auto res = sRefMap.insert(refmap::value_type(id, e));
  return res.second ? RT_OK : RT_FAIL;
}

rtError
rtObjectCache::touch(std::string const& id, time_t now)
{
  rtError e = RT_FAIL;

  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  if (itr != sRefMap.end())
  {
    itr->second.LastUsed = now;
    e = RT_OK;
  }

  return e;
}

rtError
rtObjectCache::erase(std::string const& id)
{
  rtError e = RT_FAIL;

  std::unique_lock<std::mutex> lock(sMutex);
  auto itr = sRefMap.find(id);
  if (itr != sRefMap.end())
  {
    sRefMap.erase(itr);
    e = RT_OK;
  }

  return e;
}

rtError
rtObjectCache::removeIfOlderThan(int age, int* nRemoved)
{
  int n = 0;
  time_t now = time(nullptr);

  std::unique_lock<std::mutex> lock(sMutex);
  for (auto itr = sRefMap.begin(); itr != sRefMap.end(); ++itr)
  {
    if ((now - itr->second.LastUsed) > age)
    {
      itr = sRefMap.erase(itr);
      n++;
    }
  }

  if (nRemoved)
    *nRemoved = n;

  return RT_OK;
}


