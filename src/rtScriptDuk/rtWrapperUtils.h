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

#if 0
class rtWrapperSceneUnlocker
{
public:
  rtWrapperSceneUnlocker()
    : m_hadLock(false)
  {
    if (rtWrapperSceneUpdateHasLock())
    {
      m_hadLock = true;
      rtWrapperSceneUpdateExit();
    }
  }

  ~rtWrapperSceneUnlocker()
  {
    if (m_hadLock)
      rtWrapperSceneUpdateEnter();
  }
private:
  bool m_hadLock;
};
#endif

rtValue duk2rt(duk_context *ctx, rtWrapperError* error = NULL);
void rt2duk(duk_context *ctx, const rtValue& val);
std::string rtAllocDukIdentId();
std::string rtDukPutIdentToGlobal(duk_context *ctx, const std::string &name = "");
void rtDukDelGlobalIdent(duk_context *ctx, const std::string &name);
void rtClearAllGlobalIdents(duk_context *ctx);

#endif

