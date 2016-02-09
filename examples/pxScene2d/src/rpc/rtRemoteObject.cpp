#include "rtRemoteObject.h"
#include "rtRpcTransport.h"

rtRemoteObject::rtRemoteObject(std::string const& id, std::shared_ptr<rtRpcTransport> const& transport)
  : m_ref_count(0)
  , m_id(id)
  , m_transport(transport)
{
  m_transport->keep_alive(id);
}

rtRemoteObject::~rtRemoteObject()
{
}

rtError
rtRemoteObject::Get(char const* name, rtValue* value) const
{
  return m_transport->get(m_id, name, value);
}

rtError
rtRemoteObject::Get(uint32_t index, rtValue* value) const
{
  return m_transport->get(m_id, index, value);
}

rtError
rtRemoteObject::Set(char const* name, rtValue const* value)
{
  return m_transport->set(m_id, name, value);
}

rtError
rtRemoteObject::Set(uint32_t index, rtValue const* value)
{
  return m_transport->set(m_id, index, value);
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


rtRemoteFunction::rtRemoteFunction(std::string const& id, std::string const& name, std::shared_ptr<rtRpcTransport> const& transport)
  : m_ref_count(0)
  , m_id(id)
  , m_name(name)
  , m_transport(transport)
{
}

rtRemoteFunction::~rtRemoteFunction()
{
  Release();
}

rtError
rtRemoteFunction::Send(int argc, rtValue const* argv, rtValue* result)
{
  return m_transport->send(m_id, m_name, argc, argv, result);
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
  return n;
}
