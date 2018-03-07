#ifndef RT_OBJECT_WRAPPER_H
#define RT_OBJECT_WRAPPER_H

#include "rtWrapperUtils.h"

namespace rtScriptDukUtils
{

class rtObjectWrapper : public rtWrapper<rtObjectRef, rtObjectWrapper>
{
public:
  rtObjectWrapper(const rtObjectRef& ref);
  virtual ~rtObjectWrapper();

  void dispose()
  {
  }

public:
  static void createFromObjectReference(duk_context *ctx, const rtObjectRef& ref);
};

class jsObjectWrapper : public rtIObject
{
public:
  jsObjectWrapper(duk_context *ctx, const std::string &name, bool isArray);
  virtual ~jsObjectWrapper();

  virtual unsigned long AddRef();
  virtual unsigned long Release();
  virtual unsigned long getRefCount() const
    { return mRefCount; }

  static const char* kIsJavaScriptObjectWrapper;
  static bool isJavaScriptObjectWrapper(const rtObjectRef& obj);

  virtual rtError Get(const char* name, rtValue* value) const;
  virtual rtError Get(uint32_t i, rtValue* value) const;
  virtual rtError Set(const char* name, const rtValue* value);
  virtual rtError Set(uint32_t i, const rtValue* value);
  virtual rtMethodMap* getMap() const { return NULL;  }

  void pushDukWrappedObject();

private:
  rtError getAllKeys(rtValue* value) const;

  bool dukHasProp(const std::string &name) const;
  rtValue dukGetProp(const std::string &name, rtWrapperError *error = NULL) const;
  rtValue dukGetProp(uint32_t i, rtWrapperError *error = NULL) const;

private:
  unsigned long mRefCount;
  bool mIsArray;

  duk_context *mDukCtx;
  std::string  mDukName;
};

} //namespace rtScriptDukUtils

#endif
