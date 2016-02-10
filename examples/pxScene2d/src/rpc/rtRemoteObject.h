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

class rtRemoteFunction : public rtIFunction
{
public:
  rtRemoteFunction(std::string const& id, std::string const& name,
    std::shared_ptr<rtRpcTransport> const& transport);

  virtual ~rtRemoteFunction();

  virtual unsigned long AddRef();
  virtual unsigned long Release();
  virtual rtError Send(int numArgs, const rtValue* args, rtValue* result);

private:
  rtAtomic                          m_ref_count;
  std::string                       m_id;
  std::string                       m_name;
  std::shared_ptr<rtRpcTransport>   m_transport;
};

#endif
