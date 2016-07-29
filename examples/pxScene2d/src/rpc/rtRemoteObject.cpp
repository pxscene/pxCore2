#include "rtRemoteObject.h"
#include "rtRemoteClient.h"

rtRemoteObject::rtRemoteObject(std::string const& id, std::shared_ptr<rtRemoteClient> const& client)
  : m_ref_count(0)
  , m_id(id)
  , m_rpc_client(client)
{
  m_rpc_client->keepAlive(id);
}

rtRemoteObject::~rtRemoteObject()
{
  m_rpc_client->removeKeepAlive(m_id);
  Release();
  // TODO: send deref here
}

rtError
rtRemoteObject::Get(char const* name, rtValue* value) const
{
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;

  rtJsonDocPtr msg(new rapidjson::Document());
  msg->SetObject();
  msg->AddMember(kFieldNameMessageType, kMessageTypeGetByNameRequest, msg->GetAllocator());
  msg->AddMember(kFieldNamePropertyName, std::string(name), msg->GetAllocator());
  msg->AddMember(kFieldNameCorrelationKey, rtMessage_GetNextCorrelationKey(), msg->GetAllocator());

  rtError e = m_rpc_client->send(msg);
  if (e != RT_OK)
    return e;

  // TODO: wait for response

  assert(false);

  return e;
}

rtError
rtRemoteObject::Get(uint32_t index, rtValue* value) const
{
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;

  // if we're single-threaded, register with m_env for callback
  // with our correlation key
  rtCorrelationKey const k = rtMessage_GetNextCorrelationKey();

  rtJsonDocPtr msg(new rapidjson::Document());
  msg->SetObject();
  msg->AddMember(kFieldNameMessageType, kMessageTypeGetByNameRequest, msg->GetAllocator());
  msg->AddMember(kFieldNameCorrelationKey, k, msg->GetAllocator());
  msg->AddMember(kFieldNamePropertyIndex, index, msg->GetAllocator());

  rtError e = m_rpc_client->send(msg);
  if (e != RT_OK)
    return e;

  assert(false);

  #if 0
  auto itr = res->FindMember(kFieldNameValue);
  if (itr == res->MemberEnd())
  {
    rtLogWarn("response doesn't contain: %s", kFieldNameValue);
    return RT_FAIL;
  }

  e = rtValueReader::read(value, itr->value, shared_from_this());
  if (e != RT_OK)
  {
    rtLogWarn("failed to read value from response");
    return e;
  }

  return rtMessage_GetStatusCode(*res);
  #endif

  return e;

}

rtError
rtRemoteObject::Set(char const* name, rtValue const* value)
{
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;
  return m_rpc_client->set(m_id, name, *value);
}

rtError
rtRemoteObject::Set(uint32_t index, rtValue const* value)
{
  if (value == nullptr)
    return RT_ERROR_INVALID_ARG;
  return m_rpc_client->set(m_id, index, *value);
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
