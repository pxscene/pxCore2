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

#ifndef __RT_REMOTE_FUNCTION_H__
#define __RT_REMOTE_FUNCTION_H__

#include <rtObject.h>
#include <memory>
#include <string>

class rtRemoteClient;
class rtRemoteEnvironment;

class rtRemoteFunction : public rtIFunction
{
public:
  rtRemoteFunction(std::string const& id, std::string const& name,
    std::shared_ptr<rtRemoteClient> const& transport);

  virtual ~rtRemoteFunction();

  virtual unsigned long AddRef();
  virtual unsigned long Release();
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result);

  inline std::string const& getId() const
    { return m_id; }

  inline std::string const& getName() const
    { return m_name; }

private:
  rtAtomic                          m_ref_count;
  std::string                       m_id;
  std::string                       m_name;
  std::shared_ptr<rtRemoteClient>   m_client;
  uint32_t                          m_timeout;
};

#endif
