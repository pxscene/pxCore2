#ifndef __RT_REMOTE_OBJECT_H__
#define __RT_REMOTE_OBJECT_H__

#include <rtObject.h>
#include <memory>
#include <string>

class rtRpcClient;

class rtRemoteObject : public rtIObject
{
public:
  rtRemoteObject(std::string const& id, std::shared_ptr<rtRpcClient> const& transport);
  virtual ~rtRemoteObject();

  virtual rtError Get(char const* name, rtValue* value) const;
  virtual rtError Get(uint32_t index, rtValue* value) const;
  virtual rtError Set(char const* name, rtValue const* value);
  virtual rtError Set(uint32_t index, rtValue const* value);

  virtual unsigned long AddRef();
  virtual unsigned long Release();

  inline std::string const& id() const
    { return m_id; }

private:
  rtAtomic                          m_ref_count;
  std::string                       m_id;
  std::shared_ptr<rtRpcClient>      m_transport;
};

#endif
