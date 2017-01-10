#ifndef __RT_OBJECT_CACHE_H__
#define __RT_OBJECT_CACHE_H__

#include <string>

#include <assert.h>
#include <rtError.h>
#include <rtObject.h>

class rtRemoteEnvironment;

class rtRemoteObjectCache
{
public:
  rtRemoteObjectCache(rtRemoteEnvironment* env)
    : m_env(env)
  { }

  rtObjectRef findObject(std::string const& id);
  rtFunctionRef findFunction(std::string const& id);
  rtError insert(std::string const& id, rtObjectRef const& ref);
  rtError insert(std::string const& id, rtFunctionRef const& ref);
  rtError touch(std::string const& id, time_t now);
  rtError erase(std::string const& id);
  rtError markForRemoval(std::string const& id);
  rtError removeUnused();
  rtError clear();

private:
  rtRemoteEnvironment* m_env;
};

#endif
