#include "rtRemoteObject.h"
#include "rtRemoteClient.h"

rtRemoteObject::rtRemoteObject(std::string const& id, std::shared_ptr<rtRemoteClient> const& client)
  : m_ref_count(0)
  , m_id(id)
  , m_rpc_client(client)
{rtLogInfo("rtRemoteObject::rtRemoteObject");
  m_rpc_client->keepAlive(id);
}

rtRemoteObject::~rtRemoteObject()
{rtLogInfo("rtRemoteObject::~rtRemoteObject");
  Release();
  // TODO: send deref here
}

rtError
rtRemoteObject::Get(char const* name, rtValue* value) const
{rtLogInfo("rtRemoteObject::Get");
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;
  return m_rpc_client->get(m_id, name, *value);
}

rtError
rtRemoteObject::Get(uint32_t index, rtValue* value) const
{rtLogInfo("rtRemoteObject::Get");
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;
  return m_rpc_client->get(m_id, index, *value);
}

rtError
rtRemoteObject::Set(char const* name, rtValue const* value)
{rtLogInfo("rtRemoteObject::Set");
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;
  return m_rpc_client->set(m_id, name, *value);
}

rtError
rtRemoteObject::Set(uint32_t index, rtValue const* value)
{rtLogInfo("rtRemoteObject::Set");
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;
  return m_rpc_client->set(m_id, index, *value);
}

rtObject::refcount_t
rtRemoteObject::AddRef()
{rtLogInfo("rtRemoteObject::AddRef");
  return rtAtomicInc(&m_ref_count);
}

rtObject::refcount_t
rtRemoteObject::Release()
{rtLogInfo("rtRemoteObject::Release");
  refcount_t n = rtAtomicDec(&m_ref_count);
  if (n == 0)
    delete this;
  return n;
}
