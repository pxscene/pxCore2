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

#ifndef __RT_OBJECT_CACHE_H__
#define __RT_OBJECT_CACHE_H__

#include <string>

#include <assert.h>
#include <rtError.h>
#include <rtObject.h>
#include <chrono>

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
  rtError touch(std::string const& id, std::chrono::steady_clock::time_point now);
  rtError erase(std::string const& id);
  rtError markUnevictable(std::string const& id, bool state);
  rtError removeUnused();
  rtError clear();

private:
  rtRemoteEnvironment* m_env;
};

#endif
