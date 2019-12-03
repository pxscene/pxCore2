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

#include <unistd.h>
#include <errno.h>

#include <algorithm>
#include <string>
#include <vector>
#include <utility>

#include <JavaScriptCore/JavaScript.h>

#include "rtScript.h"
#include "rtCore.h"
#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"

#include "rtScriptJSC.h"
#include "rtJSCMisc.h"
#include "rtJSCWrappers.h"
#include "rtJSCBindings.h"
#include "rtScriptJSCPrivate.h"

#include "rtHttpRequest.h"
#include "rtHttpResponse.h"

#ifndef ENABLE_V8_WEBSOCKET
# if defined(RTSCRIPT_SUPPORT_V8) || defined(USE_UV)
#  define ENABLE_V8_WEBSOCKET 1
# endif
#endif

#if defined(ENABLE_V8_WEBSOCKET) && ENABLE_V8_WEBSOCKET
#include "rtScriptV8/rtWebSocket.h"
#endif

extern "C" JS_EXPORT void JSSynchronousGarbageCollectForDebugging(JSContextRef);

namespace
{

class rtHttpRequestEx : public rtHttpRequest
{
public:
  rtDeclareObject(rtHttpRequestEx, rtHttpRequest);

  rtHttpRequestEx(const rtString& url)
    : rtHttpRequest(url)
  { }

  rtHttpRequestEx(const rtObjectRef& options)
    : rtHttpRequest(options)
  { }

  void onDownloadComplete(rtFileDownloadRequest* downloadRequest) override
  {
    AddRef();
    if (!downloadRequest->errorString().isEmpty()) {
      RtJSC::dispatchOnMainLoop(
        [this, errorString = downloadRequest->errorString()] () {
          mEmit.send("error", errorString);
          Release();
        });
    } else {
      rtHttpResponse* resp = new rtHttpResponse();
      resp->setStatusCode((int32_t)downloadRequest->httpStatusCode());
      resp->setErrorMessage(downloadRequest->errorString());
      resp->setHeaders(downloadRequest->headerData(), downloadRequest->headerDataSize());
      resp->setDownloadedData(downloadRequest->downloadedData(), downloadRequest->downloadedDataSize());

      rtObjectRef protectedRef = resp;
      RtJSC::dispatchOnMainLoop(
        [this, resp = resp, ref = protectedRef] () {
          mEmit.send("response", ref);
          resp->onData();
          resp->onEnd();
          Release();
        });
    }
  }
};
rtDefineObject(rtHttpRequestEx, rtHttpRequest);

rtError rtHttpGetBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);

  if (numArgs < 1) {
    rtLogError("%s: invalid args", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  rtHttpRequest* req;
  if (args[0].getType() == RT_stringType) {
    req = new rtHttpRequestEx(args[0].toString());
  }
  else {
    if (args[0].getType() != RT_objectType) {
      rtLogError("%s: invalid arg type", __FUNCTION__);
      return RT_ERROR_INVALID_ARG;
    }
    rtLogInfo("new rtHttpRequest");
    req = new rtHttpRequestEx(args[0].toObject());
  }

  if (numArgs > 1 && args[1].getType() == RT_functionType) {
    req->addListener("response", args[1].toFunction());
  }

  rtObjectRef ref = req;
  *result = ref;

  return RT_OK;
}

rtError rtWebSocketBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);
#if defined(ENABLE_V8_WEBSOCKET)
  if (numArgs < 1) {
    rtLogError("%s: invalid args", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  if (args[0].getType() != RT_objectType) {
    rtLogError("%s: invalid arg type", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  rtObjectRef ref = new rtWebSocket(args[0].toObject(), nullptr);
  *result = ref;
  return RT_OK;
#else
  rtLogError("Not supported");
  return RT_FAIL;
#endif
}

rtError rtInstallTimeout(int numArgs, const rtValue* args, rtValue* result, bool repeat)
{
  if (numArgs < 1) {
    rtLogError("%s: invalid args", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  if (args[0].getType() != RT_functionType) {
    rtLogError("%s: invalid arg type", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  double interval = 0;
  if (numArgs >= 2)
    interval = args[1].toDouble();

  rtFunctionRef timeoutCb = args[0].toFunction();

  std::vector<rtValue> timeoutArgs;
  if (numArgs > 2) {
    timeoutArgs.reserve(numArgs - 2);
    for (int i = 2; i < numArgs; ++i)
      timeoutArgs.push_back(args[i]);
  }

  uint32_t tag = RtJSC::installTimeout(interval, repeat,
    [timeoutCbRef = std::move(timeoutCb), timeoutArgs = std::move(timeoutArgs)] () mutable {
      rtError rc = timeoutCbRef.rtFunctionBase::Send(timeoutArgs.size(), timeoutArgs.data());
      if (rc != RT_OK) {
        rtLogError("timer callback send failed, rc = %d", rc);
        return 1;
      }
      return 0;
    });

  if (result)
    *result = tag;

  return RT_OK;
}

rtError rtSetItervalBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);
  return rtInstallTimeout(numArgs, args, result, true);
}

rtError rtSetTimeoutBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);
  return rtInstallTimeout(numArgs, args, result, false);
}

rtError rtClearTimeoutBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);
  if (numArgs < 1) {
    rtLogError("%s: invalid args", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }
  if (args[0].isEmpty()) {
    rtLogWarn("%s: cannot remove time for 'null' or 'undefined' tag", __FUNCTION__);
    return RT_OK;
  }
  uint32_t tag = args[0].toUInt32();
  RtJSC::clearTimeout(tag);
  return RT_OK;
}

JSContextGroupRef globalContextGroup()
{
  static JSContextGroupRef gGroupRef = JSContextGroupCreate();
  return gGroupRef;
}

}  // namespace

namespace RtJSC
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class rtJSCContext: public RefCounted<rtIScriptContext>
{
public:
  rtJSCContext();
  virtual ~rtJSCContext();

  rtError add(const char *name, rtValue  const& val);
  rtValue get(const char *name);
  bool    has(const char *name);

  rtError runScript(const char *script, rtValue* retVal = nullptr, const char *args = nullptr);
  rtError runFile  (const char *file,   rtValue* retVal = nullptr, const char *args = nullptr);

private:
  rtError evaluateScript(const char *script, const char *name, rtValue* retVal = nullptr, const char *args = nullptr);

  JSContextGroupRef m_contextGroup { nullptr };
  JSGlobalContextRef m_context { nullptr };
  rtRef<rtJSCContextPrivate> m_priv;

  rtRef<rtFunctionCallback> m_httpGetBinding;
  rtRef<rtFunctionCallback> m_webScoketBinding;
  rtRef<rtFunctionCallback> m_setTimeoutBinding;
  rtRef<rtFunctionCallback> m_clearTimeoutBinding;
  rtRef<rtFunctionCallback> m_setIntervalBinding;
  rtRef<rtFunctionCallback> m_clearIntervalBinding;
};

rtJSCContext::rtJSCContext()
{
  rtLogInfo("%s", __FUNCTION__);
  m_contextGroup = JSContextGroupRetain(globalContextGroup());
  m_context = JSGlobalContextCreateInGroup(m_contextGroup, nullptr);
  m_priv = rtJSCContextPrivate::create(m_context);

  injectBindings(m_context);

  m_httpGetBinding = new rtFunctionCallback(rtHttpGetBinding, nullptr);
  m_webScoketBinding = new rtFunctionCallback(rtWebSocketBinding, nullptr);
  m_setTimeoutBinding = new rtFunctionCallback(rtSetTimeoutBinding, nullptr);
  m_clearTimeoutBinding = new rtFunctionCallback(rtClearTimeoutBinding, nullptr);
  m_setIntervalBinding = new rtFunctionCallback(rtSetItervalBinding, nullptr);
  m_clearIntervalBinding = new rtFunctionCallback(rtClearTimeoutBinding, nullptr);

  add("httpGet", m_httpGetBinding.getPtr());
  add("webscoketGet", m_webScoketBinding.getPtr());
  add("setTimeout", m_setTimeoutBinding.getPtr());
  add("clearTimeout", m_clearTimeoutBinding.getPtr());
  add("setInterval", m_setIntervalBinding.getPtr());
  add("clearInterval", m_clearIntervalBinding.getPtr());
}

rtJSCContext::~rtJSCContext()
{
  rtLogInfo("%s begin", __FUNCTION__);

//  RtJSC::dispatchOnMainLoop([m_priv=m_priv,m_context=m_context,m_contextGroup=m_contextGroup] {
//  static JSStringRef codeStr = JSStringCreateWithUTF8CString("console.clear(); delete global.console");
//  JSEvaluateScript(m_context, codeStr, nullptr, nullptr, 0, nullptr);

  m_priv->releaseAllProtected();

  // schedule GC
  JSGarbageCollect(m_context);
  // JSSynchronousGarbageCollectForDebugging(m_context);
  JSGlobalContextRelease(m_context);
  JSContextGroupRelease(m_contextGroup);

//  });

  rtLogInfo("%s end", __FUNCTION__);
}

rtError rtJSCContext::add(const char *name, rtValue const& val)
{
  JSStringRef jsName = JSStringCreateWithUTF8CString(name);
  JSValueRef jsVal = rtToJs(m_context, val);
  JSObjectRef globalObj = JSContextGetGlobalObject(m_context);
  JSValueRef exception = nullptr;
  JSObjectSetProperty(m_context, globalObj, jsName, jsVal, kJSPropertyAttributeDontEnum, &exception);
  JSStringRelease(jsName);

  if (exception) {
    JSStringRef exceptStr = JSValueToStringCopy(m_context, exception, nullptr);
    rtString errorStr = jsToRtString(exceptStr);
    JSStringRelease(exceptStr);
    rtLogError("Failed to add to rtScript context, error='%s'\n", errorStr.cString());
    return RT_FAIL;
  }
  return RT_OK;
}

rtValue rtJSCContext::get(const char *name)
{
  if (!name)
    return {};

  JSStringRef jsName = JSStringCreateWithUTF8CString(name);
  JSObjectRef globalObj = JSContextGetGlobalObject(m_context);
  JSValueRef exception = nullptr;
  rtValue retVal;

  do {
    JSValueRef valueRef = JSObjectGetProperty(m_context, globalObj, jsName, &exception);
    if (exception)
      break;
    jsToRt(m_context, valueRef, retVal, &exception);
    if (exception)
      break;
  } while(0);
  JSStringRelease(jsName);

  if (exception) {
    JSStringRef exceptStr = JSValueToStringCopy(m_context, exception, nullptr);
    rtString errorStr = jsToRtString(exceptStr);
    JSStringRelease(exceptStr);
    rtLogError("Failed to get %s from rtScript context, error='%s'\n", name, errorStr.cString());
    return rtValue();
  }

  return retVal;
}

bool rtJSCContext::has(const char *name)
{
  if (!name)
    return {};
  JSStringRef jsName = JSStringCreateWithUTF8CString(name);
  JSObjectRef globalObj = JSContextGetGlobalObject(m_context);
  bool ret = JSObjectHasProperty(m_context, globalObj, jsName);
  JSStringRelease(jsName);
  return ret;
}

rtError rtJSCContext::evaluateScript(const char* script, const char* name, rtValue* retVal, const char *args)
{
  rtLogInfo("rtJSCContext::evaluateScript name=%s", name);

  JSValueRef exception = nullptr;
  JSStringRef codeStr = JSStringCreateWithUTF8CString(script);
  JSObjectRef globalObj = JSContextGetGlobalObject(m_context);
  JSStringRef fileStr = nullptr;

  if (name) {
    static int evalCount = 0;
    std::string ctxNameStr = std::to_string(++evalCount) + ": " + name;
    fileStr = JSStringCreateWithUTF8CString(ctxNameStr.c_str());
    JSGlobalContextSetName(m_context, fileStr);
  } else {
    fileStr = JSStringCreateWithUTF8CString("rtJSCContext::evaluateScript");
  }

  JSValueRef result = JSEvaluateScript(m_context, codeStr, globalObj, fileStr, 0, &exception);
  JSStringRelease(codeStr);
  JSStringRelease(fileStr);

  if (exception) {
    JSStringRef exceptStr = JSValueToStringCopy(m_context, exception, nullptr);
    rtString errorStr = jsToRtString(exceptStr);
    JSStringRelease(exceptStr);
    rtLogError("Failed to eval, error='%s'", errorStr.cString());
    return RT_FAIL;
  }

  rtError ret = RT_OK;
  if (retVal) {
    if(result)
      ret = jsToRt(m_context, result, *retVal, nullptr);
    else
      *retVal = rtValue();
  }
  return ret;
}

rtError rtJSCContext::runScript(const char* script, rtValue* retVal, const char *args)
{
  return evaluateScript(script, nullptr, retVal, args);
}

rtError rtJSCContext::runFile(const char *file, rtValue* retVal, const char* args)
{
  if (!file) {
    rtLogError(" %s  ... no script given.",__PRETTY_FUNCTION__);
    return RT_FAIL;
  }

  std::string codeStr = readFile(file);
  if(codeStr.empty()) {
    rtLogError(" %s  ... load error / not found.",__PRETTY_FUNCTION__);
    return RT_FAIL;
  }

  return evaluateScript(codeStr.c_str(), file, retVal, args);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class rtScriptJSC: public RefCounted<rtIScript>
{
public:
  rtScriptJSC();
  virtual ~rtScriptJSC();

  rtError init() override;
  rtError term() override;
  rtString engine() override { return "JavaScriptCore"; }
  rtError pump() override;
  rtError createContext(const char *lang, rtScriptContextRef& ctx) override;
  rtError collectGarbage() override;
  void* getParameter(rtString param) override;
};

rtScriptJSC::rtScriptJSC()
{
  RtJSC::initMainLoop();
}

rtError rtScriptJSC::init()
{
  return RT_OK;
}

rtScriptJSC::~rtScriptJSC()
{
  rtLogInfo("%s", __FUNCTION__);
}

rtError rtScriptJSC::pump()
{
  RtJSC::pumpMainLoop();
  return RT_OK;
}

rtError rtScriptJSC::collectGarbage()
{
  return RT_OK;
}

void* rtScriptJSC::getParameter(rtString param)
{
  return nullptr;
}

rtError rtScriptJSC::term()
{
  return RT_OK;
}

rtError rtScriptJSC::createContext(const char *lang, rtScriptContextRef& ctx)
{
  ctx = static_cast<rtIScriptContext*>(new rtJSCContext());
  return RT_OK;
}

}  // namespace RtJSC

rtError createScriptJSC(rtScriptRef& script)
{
  script = new RtJSC::rtScriptJSC();
  return RT_OK;
}
