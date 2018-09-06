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

#ifndef RT_WRAPPER_UTILS
#define RT_WRAPPER_UTILS

#include <rtError.h>
#include <rtObject.h>
#include <rtString.h>
#include <rtValue.h>

#include "rtScript.h"

#include <stdarg.h>
#include <string>
#include <map>
#include <memory>

extern "C" {
#include "duv.h"
}

namespace rtScriptDukUtils
{

#if 0
bool rtIsMainThreadDuk();
#endif

class rtWrapperError
{
public:
  rtWrapperError() { }
  rtWrapperError(const char* errorMessage)
    : mMessage(errorMessage) { }

  inline bool hasError() const
    { return !mMessage.empty(); }

  void setMessage(const char* errorMessage)
    { mMessage = errorMessage; }

private:
  std::string mMessage;
};


template<typename TRef, typename TWrapper>
class rtWrapper
{
protected:
  rtWrapper(const TRef& ref) : mWrappedObject(ref)
  {
  }

  virtual ~rtWrapper(){ }

protected:
  TRef mWrappedObject;
};

rtValue duk2rt(duk_context *ctx, rtWrapperError* error = NULL);
void rt2duk(duk_context *ctx, const rtValue& val);
std::string rtAllocDukIdentId();
std::string rtDukPutIdentToGlobal(duk_context *ctx, const std::string &name = "");
void rtDukDelGlobalIdent(duk_context *ctx, const std::string &name);

} //namespace rtScriptDukUtils

#endif

