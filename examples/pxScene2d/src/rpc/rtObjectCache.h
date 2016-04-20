#ifndef __RT_OBJECT_CACHE_H__
#define __RT_OBJECT_CACHE_H__

#include <string>
#include <rtError.h>
#include <rtObject.h>

class rtObjectCache
{
public:
  static rtObjectRef findObject(std::string const& id);
  static rtFunctionRef findFunction(std::string const& id);
  static rtError insert(std::string const& id, rtObjectRef const& ref, int maxAge);
  static rtError insert(std::string const& id, rtFunctionRef const& ref, int maxAge);
  static rtError touch(std::string const& id, time_t now);
  static rtError erase(std::string const& id);
  static rtError removeUnused();

private:
  rtObjectCache() { }
};

#endif
