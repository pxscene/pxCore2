#ifndef __RT_REMOTE_OBJECT_H__
#define __RT_REMOTE_OBJECT_H__

#include <rtObject.h>

class rtRemoteTransport
{
public:
};

class rtValueReader
{
};

class rtValueWriter
{
};

class rtRemoteObject : public rtIObject
{
public:
  rtRemoteObject();
  virtual ~rtRemoteObject();

  virtual rtError Get(char const* name, rtValue* value) const;
  virtual rtError Get(uint32_t index, rtValue* value) const;
  virtual rtError Set(char const* name, rtValue const* value);
  virtual rtError Set(uint32_t index, rtValue const* value);

  virtual refcount_t AddRef();
  virtual refcount_t Release();

private:
  refcount_t mRefCount;
};

#endif
