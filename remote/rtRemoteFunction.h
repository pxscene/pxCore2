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
