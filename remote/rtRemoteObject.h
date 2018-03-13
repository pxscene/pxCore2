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

#ifndef __RT_REMOTE_OBJECT_H__
#define __RT_REMOTE_OBJECT_H__

#include <rtObject.h>
#include <memory>
#include <string>

class rtRemoteClient;

class rtRemoteObject : public rtIObject
{
public:
  rtRemoteObject(std::string const& id, std::shared_ptr<rtRemoteClient> const& transport);
  virtual ~rtRemoteObject();

  virtual rtError Get(char const* name, rtValue* value) const;
  virtual rtError Get(uint32_t index, rtValue* value) const;
  virtual rtError Set(char const* name, rtValue const* value);
  virtual rtError Set(uint32_t index, rtValue const* value);

  virtual unsigned long AddRef();
  virtual unsigned long Release();
  virtual rtMethodMap* getMap() const { return NULL;  }
  inline std::string const& getId() const
    { return m_id; }

private:
  rtAtomic                          m_ref_count;
  std::string                       m_id;
  std::shared_ptr<rtRemoteClient>   m_client;
};

#endif
