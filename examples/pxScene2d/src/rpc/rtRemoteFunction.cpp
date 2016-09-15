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
  if (!strcmp(id.c_str(), "global"))
  {
    m_client->keepAlive(m_name);
  }
}

rtRemoteFunction::~rtRemoteFunction()
{
  if (!strcmp(m_id.c_str(), "global"))
  {
    m_client->removeKeepAlive(m_name);
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
