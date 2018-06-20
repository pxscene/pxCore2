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

#include "rtRemoteObject.h"
#include "rtRemoteClient.h"
#include "rtError.h"

rtRemoteObject::rtRemoteObject(std::string const& id, std::shared_ptr<rtRemoteClient> const& client)
  : m_ref_count(0)
  , m_id(id)
  , m_client(client)
{
  m_client->registerKeepAliveForObject(id);
}

rtRemoteObject::~rtRemoteObject()
{
  m_client->removeKeepAliveForObject(m_id);
  Release();
  // TODO: send deref here
}

rtError
rtRemoteObject::Get(char const* name, rtValue* value) const
{
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;

  if (name == nullptr)
    return RT_ERROR_INVALID_ARG;

  return m_client->sendGet(m_id, name, *value);
}

rtError
rtRemoteObject::Get(uint32_t index, rtValue* value) const
{
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;

  return m_client->sendGet(m_id, index, *value);
}

rtError
rtRemoteObject::Set(char const* name, rtValue const* value)
{
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;

  return m_client->sendSet(m_id, name, *value);
}

rtError
rtRemoteObject::Set(uint32_t index, rtValue const* value)
{
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;

  return m_client->sendSet(m_id, index, *value);
}

rtObject::refcount_t
rtRemoteObject::AddRef()
{
  return rtAtomicInc(&m_ref_count);
}

rtObject::refcount_t
rtRemoteObject::Release()
{
  refcount_t n = rtAtomicDec(&m_ref_count);
  if (n == 0)
    delete this;
  return n;
}
