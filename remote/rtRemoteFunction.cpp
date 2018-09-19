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

#include "rtRemoteFunction.h"
#include "rtRemoteClient.h"
#include "rtRemoteConfig.h"
#include "rtRemoteEnvironment.h"

rtRemoteFunction::rtRemoteFunction(std::string const& id, std::string const& name, std::shared_ptr<rtRemoteClient> const& client)
  : m_ref_count(0)
  , m_id(id)
  , m_name(name)
  , m_client(client)
  , m_timeout(client->getEnvironment()->Config->environment_request_timeout())
{
  std::hash<std::string> hashFn;
  m_Hash = hashFn(name);

  if (!strcmp(id.c_str(), "global"))
  {
    m_client->registerKeepAliveForObject(m_name);
  }
}

rtRemoteFunction::~rtRemoteFunction()
{
  if (!strcmp(m_id.c_str(), "global"))
  {
    m_client->removeKeepAliveForObject(m_name);
  }
  Release();
}

rtError
rtRemoteFunction::Send(int argc, rtValue const* argv, rtValue* result)
{
  rtValue res;
  rtError e = m_client->sendCall(m_id, m_name, argc, argv, res);
  if (e == RT_OK)
  {
    if (result != nullptr)
      *result = res;
  }
  return e;
}

unsigned long
rtRemoteFunction::AddRef()
{
  return rtAtomicInc(&m_ref_count);
}

unsigned long
rtRemoteFunction::Release()
{
  unsigned long n = rtAtomicDec(&m_ref_count);
  if (n == 0)
    delete this;

  // TODO: send deref here
  return n;
}

size_t
rtRemoteFunction::hash()
{
  return m_Hash;
}

void
rtRemoteFunction::setHash(size_t hash)
{
  m_Hash = hash;
}
