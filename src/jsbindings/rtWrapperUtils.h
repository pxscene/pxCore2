#ifndef RT_WRAPPER_UTILS
#define RT_WRAPPER_UTILS

#include <rtError.h>
#include <rtObject.h>
#include <rtString.h>
#include <rtValue.h>

#include <stdarg.h>
#include <string>
#include <map>
#include <memory>

extern "C" {
#include "duv.h"
}

bool rtIsMainThread();

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


bool rtWrapperSceneUpdateHasLock();
void rtWrapperSceneUpdateEnter();
void rtWrapperSceneUpdateExit();

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

rtValue duk2rt(duk_context *ctx, rtWrapperError* error = NULL);
void rt2duk(duk_context *ctx, const rtValue& val);
std::string rtAllocDukIdentId();
std::string rtDukPutIdentToGlobal(duk_context *ctx, const std::string &name = "");

#endif

