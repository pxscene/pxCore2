#include "rtRemoteObject.h"

rtRemoteObject::rtRemoteObject()
  : mRefCount(0)
{
}

rtRemoteObject::~rtRemoteObject()
{
}

rtError
rtRemoteObject::Get(char const* name, rtValue* value) const
{
  return 0;
}

rtError
rtRemoteObject::Get(uint32_t index, rtValue* value) const
{
  return 0;
}

rtError
rtRemoteObject::Set(char const* name, rtValue const* value)
{
  return 0;
}

rtError
rtRemoteObject::Set(uint32_t index, rtValue const* value)
{
  return 0;
}

rtObject::refcount_t
rtRemoteObject::AddRef()
{
  return rtAtomicInc(&mRefCount);
}

rtObject::refcount_t
rtRemoteObject::Release()
{
  refcount_t n = rtAtomicDec(&mRefCount);
  if (n == 0)
    delete this;
  return n;
}
