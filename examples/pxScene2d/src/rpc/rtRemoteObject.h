#ifndef __RT_REMOTE_OBJECT_H__
#define __RT_REMOTE_OBJECT_H__

#include <rtObject.h>
#include <memory>
#include <string>

class rtRpcTransport;

class rtRemoteObject : public rtIObject
{
public:
  rtRemoteObject(std::string const& id, std::shared_ptr<rtRpcTransport> const& transport);
  virtual ~rtRemoteObject();

  virtual rtError Get(char const* name, rtValue* value) const;
  virtual rtError Get(uint32_t index, rtValue* value) const;
  virtual rtError Set(char const* name, rtValue const* value);
  virtual rtError Set(uint32_t index, rtValue const* value);

  virtual refcount_t AddRef();
  virtual refcount_t Release();

  inline std::string const& id() const
    { return m_id; }

private:
  refcount_t                        m_ref_count;
  std::string                       m_id;
  std::shared_ptr<rtRpcTransport>   m_transport;
};

#endif
