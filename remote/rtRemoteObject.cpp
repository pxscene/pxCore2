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

  auto iter = m_functions.find(name);
  if (iter != m_functions.end())
  {
    *value = iter->second;
    return RT_OK;
  }

  rtError rc = m_client->sendGet(m_id, name, *value);

  if (rc == RT_OK && value->getType() == RT_functionType)
    m_functions[name] = value->toFunction();

  return rc;
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
