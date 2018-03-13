/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef RT_OBJECT_WRAPPER_H
#define RT_OBJECT_WRAPPER_H

#include "rtWrapperUtils.h"

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



#endif
