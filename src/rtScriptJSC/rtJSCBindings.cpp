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

#include "rtJSCBindings.h"
#include "rtJSCMisc.h"
#include "rtScriptJSCPrivate.h"
#include "rtLog.h"

#include "pxTimer.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <list>
#include <cassert>
#include <vector>

namespace RtJSC {

static JSValueRef noopCallback(JSContextRef ctx, JSObjectRef fun, JSObjectRef, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
  rtLogDebug("no-op");
  return JSValueMakeUndefined(ctx);
}

static JSValueRef exitCallback(JSContextRef ctx, JSObjectRef fun, JSObjectRef, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
  RtJSC::dispatchOnMainLoop([] () { _exit(0); });
  return JSValueMakeUndefined(ctx);
}

static JSValueRef hrTimeCallback(JSContextRef ctx, JSObjectRef, JSObjectRef, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
  auto dur = std::chrono::high_resolution_clock::now().time_since_epoch();
  uint64_t hrtime = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
  int nanoseconds = hrtime % (int)1e9;
  int seconds = hrtime / 1e9;
  JSValueRef args[] = { JSValueMakeNumber(ctx, seconds), JSValueMakeNumber(ctx, nanoseconds) };
  return JSObjectMakeArray(ctx, 2, args, exception);
}

static JSValueRef readFileCallback(JSContextRef ctx, JSObjectRef, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
  if (argumentCount != 2)
    return JSValueMakeUndefined(ctx);

  JSValueRef result = nullptr;
  do {
    JSStringRef filePath = JSValueToStringCopy(ctx, arguments[0], exception);
    if (exception && *exception)
      break;

    rtString path = jsToRtString(filePath);
    JSStringRelease(filePath);

    JSObjectRef callbackObj = JSValueToObject(ctx, arguments[1], exception);
    if (exception && *exception)
      break;

    #if 0
    int retCode = -1;
    JSValueRef retArr = JSValueMakeNull(ctx);
    if ((retCode = access(path.cString(), R_OK)) == 0) {
      auto* contents = new std::vector<uint8_t> { readBinFile(path.cString()) };
      retArr = JSObjectMakeArrayBufferWithBytesNoCopy(
        ctx, contents->data(), contents->size(),
        [](void* bytes, void* deallocatorContext) {
          auto* contents = reinterpret_cast<std::vector<uint8_t>*>(deallocatorContext);
          assert(contents->data() == bytes);
          delete contents;
        }, contents, exception);

      if (exception && *exception)
        break;
    }
    JSValueRef args[] = { JSValueMakeNumber(ctx, retCode), retArr };
    result = JSObjectCallAsFunction(ctx, callbackObj, thisObject, 2, args, exception);
    #else
    int retCode = -1;
    JSStringRef retStr = nullptr;
    if ((retCode = access(path.cString(), R_OK)) == 0) {
      retStr = JSStringCreateWithUTF8CString(readFile(path.cString()).c_str());
    } else {
      retStr = JSStringCreateWithUTF8CString("");
    }

    JSValueRef args[] = { JSValueMakeNumber(ctx, retCode), JSValueMakeString(ctx, retStr) };
    result = JSObjectCallAsFunction(ctx, callbackObj, thisObject, 2, args, exception);
    JSStringRelease(retStr);
    #endif
  } while (0);

  if (exception && *exception) {
    printException(ctx, *exception);
    return JSValueMakeUndefined(ctx);
  }

  return result;
}

static JSValueRef requireCallback(JSContextRef ctx, JSObjectRef, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
  if (argumentCount != 1)
      return JSValueMakeNull(ctx);

  static const auto resolveModulePath = [](const rtString &name, rtString &data) -> bool
  {
    std::list<rtString> dirs;
    std::list<rtString> endings;
    bool found = false;
    rtString path;

    dirs.push_back(""); // this dir
    dirs.push_back("jsc_modules/");

    endings.push_back(".js");

    std::list<rtString>::const_iterator it, jt;
    for (it = dirs.begin(); !found && it != dirs.end(); ++it) {
      rtString s = *it;
      if (!s.isEmpty() && !s.endsWith("/"))
        s.append("/");
      s.append(name.beginsWith("./") ? name.substring(2) : name);
      for (jt = endings.begin(); !found && jt != endings.end(); ++jt) {
        path = s;
        if (!path.endsWith((*jt).cString()))
          path.append(*jt);
        found = fileExists(path.cString());
      }
    }

    if (found)
      data = path;
    return found;
  };

  do {
    JSStringRef reqArgStr = JSValueToStringCopy(ctx, arguments[0], exception);
    if (exception && *exception)
      break;

    rtString moduleName = jsToRtString(reqArgStr);
    rtString path;
    if (!resolveModulePath(moduleName, path)) {
      JSStringRelease(reqArgStr);
      rtLogError("Module '%s' not found", moduleName.cString());
      break;
    }

    JSGlobalContextRef globalCtx = JSContextGetGlobalContext(ctx);
    rtJSCContextPrivate* priv = rtJSCContextPrivate::fromCtx(globalCtx);
    if (!priv) {
      rtLogError(" %s  ... no priv object.",__PRETTY_FUNCTION__);
      break;
    }

    if (JSObjectRef moduleObj = priv->findModule(path)) {
      JSStringRelease(reqArgStr);
      return moduleObj;
    }

    rtLogInfo("Loading %s", path.cString());
    std::string codeStr = readFile(path.cString());
    if (codeStr.empty()) {
      JSStringRelease(reqArgStr);
      rtLogError(" %s  ... load error / not found.",__PRETTY_FUNCTION__);
      break;
    }

    codeStr =
        "(function(){ let m = {}; m.exports = {}; \n"
        "  (function(module, exports){\n"
        + codeStr +
        "  \n}).call(undefined, m, m.exports); return m;})()";

    JSStringRef jsstr = JSStringCreateWithUTF8CString(codeStr.c_str());
    JSValueRef module = JSEvaluateScript(globalCtx, jsstr, nullptr, reqArgStr, 0, exception);
    JSStringRelease(jsstr);
    JSStringRelease(reqArgStr);

    if (exception && *exception) {
      JSStringRef exceptStr = JSValueToStringCopy(globalCtx, *exception, nullptr);
      rtString errorStr = jsToRtString(exceptStr);
      JSStringRelease(exceptStr);
      rtLogError("Failed to eval, \n\terror='%s'\n\tmodule=%s\n\tscript='...'", errorStr.cString(), path.cString());
      break;
    }

    JSObjectRef moduleObj = JSValueToObject(globalCtx, module, exception);
    if (exception && *exception) {
      JSStringRef exceptStr = JSValueToStringCopy(globalCtx, *exception, nullptr);
      rtString errorStr = jsToRtString(exceptStr);
      JSStringRelease(exceptStr);
      rtLogError("Failed to convert module to object, \n\terror='%s'\n\tmodule=%s", errorStr.cString(), path.cString());
      break;
    }

    static JSStringRef exportsStr = JSStringCreateWithUTF8CString("exports");
    JSValueRef exportsVal = JSObjectGetProperty(globalCtx, moduleObj, exportsStr, exception);
    if (exception && *exception) {
      JSStringRef exceptStr = JSValueToStringCopy(globalCtx, *exception, nullptr);
      rtString errorStr = jsToRtString(exceptStr);
      JSStringRelease(exceptStr);
      rtLogError("Failed to get exports module to object, \n\terror='%s'\n\tmodule=%s", errorStr.cString(), path.cString());
      break;
    }

    JSObjectRef exportsObj = JSValueToObject(globalCtx, exportsVal, exception);
    if (exception && *exception) {
      printException(globalCtx, *exception);
      break;
    }
    priv->addToModuleCache(path, globalCtx, exportsObj);

    return exportsVal;
  } while(0);

  return JSValueMakeNull(ctx);
}

static void markJSContext(JSContextRef ctx, JSObjectRef globalObj, JSValueRef *exception)
{
  static JSStringRef jsName = JSStringCreateWithUTF8CString("_isJSC");
  JSContextRef globalCtx = JSContextGetGlobalContext(ctx);
  if (!globalObj)
    globalObj = JSContextGetGlobalObject(globalCtx);
  JSObjectSetProperty(globalCtx, globalObj, jsName, JSValueMakeBoolean(globalCtx, true), kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, exception);

  static JSStringRef globalName = JSStringCreateWithUTF8CString("global");
  JSObjectSetProperty(globalCtx, globalObj, globalName, globalObj, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, nullptr);
}

class SandboxPrivate
{
  rtJSCProtected m_protectedObj;

  SandboxPrivate(JSContextRef context, JSObjectRef obj)
    : m_protectedObj(context, obj)
  {
  }

  static void SandboxPrivate_finalize(JSObjectRef obj)
  {
    SandboxPrivate *priv = (SandboxPrivate *)JSObjectGetPrivate(obj);
    JSObjectSetPrivate(obj, nullptr);
    delete priv;
  }

  static JSClassRef privateClass()
  {
    static JSClassRef sClassRef = nullptr;
    if (!sClassRef) {
      JSClassDefinition classDef = { 0 };
      classDef.finalize = SandboxPrivate_finalize;
      sClassRef = JSClassCreate(&classDef);
    }
    return sClassRef;
  }

public:
  JSObjectRef wrapped() const { return m_protectedObj.wrapped(); }
  JSGlobalContextRef context() const { return m_protectedObj.context(); }

  static JSStringRef privateName()
  {
    static JSStringRef nameStr = JSStringCreateWithUTF8CString("__rt_sanbox_priv");
    return nameStr;
  }

  static SandboxPrivate* from(JSContextRef ctx, JSObjectRef sandboxRef, JSValueRef *exception)
  {
    JSValueRef sandboxPrivRef = JSObjectGetProperty(ctx, sandboxRef, privateName(), exception);
    if (exception && *exception)
      return nullptr;
    JSObjectRef sandboxPriv = JSValueToObject(ctx, sandboxPrivRef,  exception);
    if (exception && *exception)
      return nullptr;
    return (SandboxPrivate*)JSObjectGetPrivate(sandboxPriv);
  }

  static void init(JSContextRef ctx, JSObjectRef sandboxRef, JSContextRef sandboxCtx, JSObjectRef sandboxGlobal, JSValueRef *exception)
  {
    auto priv = new SandboxPrivate(sandboxCtx, sandboxGlobal);
    auto sandboxPriv = JSObjectMake(ctx, privateClass(), priv);
    JSObjectSetProperty(ctx, sandboxRef, privateName(), sandboxPriv,
                        kJSPropertyAttributeDontEnum | kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, exception);
  }
};

static JSValueRef runInContext(JSContextRef ctx, JSObjectRef, JSObjectRef thisobj, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
  if (argumentCount < 7)
    return JSValueMakeUndefined(ctx);

  JSValueRef result = nullptr;
  do {
    // sandbox
    JSObjectRef sandboxRef = JSValueToObject(ctx, arguments[1],  exception);
    if (exception && *exception)
      break;

    SandboxPrivate* priv = SandboxPrivate::from(ctx, sandboxRef, exception);
    if (exception && *exception)
      break;

    JSGlobalContextRef sandboxCtx = nullptr;
    JSObjectRef sandboxGlobalObj = nullptr;
    if (priv) {
      sandboxGlobalObj = priv->wrapped();
      sandboxCtx = priv->context();
    }

    if (!sandboxCtx) {
      // sandboxCtx = JSContextGetGlobalContext(ctx);
      if (exception) {
        static JSStringRef exceptionStr = JSStringCreateWithUTF8CString("No sandbox context");
        *exception = JSValueMakeString(ctx, exceptionStr);
      }
      break;
    }

    // code
    JSStringRef codeStr = JSValueToStringCopy(ctx, arguments[0], exception);
    if (exception && *exception)
      break;

    JSStringRef fileNameStr = JSValueToStringCopy(ctx, arguments[5], exception);
    if (exception && *exception) {
      JSStringRelease(codeStr);
      break;
    }
    JSGlobalContextSetName(sandboxCtx, fileNameStr);

    JSValueRef evalResult = JSEvaluateScript(sandboxCtx, codeStr, sandboxGlobalObj, fileNameStr, 0, exception);
    JSStringRelease(fileNameStr);
    JSStringRelease(codeStr);
    if (exception && *exception)
      break;

    JSObjectRef funcObj = JSValueToObject(sandboxCtx, evalResult, exception);
    if (exception && *exception)
      break;

    JSValueRef args[] = { arguments[3], arguments[4] };
    result = JSObjectCallAsFunction(sandboxCtx, funcObj, sandboxGlobalObj, 2, args, exception);
  } while (0);

  if (exception && *exception) {
    printException(ctx, *exception);
    return JSValueMakeUndefined(ctx);
  }

  return result;
}

static JSValueRef runInNewContext(JSContextRef ctx, JSObjectRef, JSObjectRef thisobj, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
  if (argumentCount < 7)
    return JSValueMakeUndefined(ctx);

  JSValueRef result = nullptr;

  JSContextGroupRef groupRef = JSContextGetGroup(JSContextGetGlobalContext(ctx));
  JSGlobalContextRef newCtx = JSGlobalContextCreateInGroup(groupRef, nullptr);

  auto priv = rtJSCContextPrivate::fromCtx(JSContextGetGlobalContext(ctx));
  assert(priv != nullptr);
  rtJSCContextPrivate::setInCtx(newCtx, priv);

  do {
    JSObjectRef newGlobalObj = JSContextGetGlobalObject(newCtx);
    markJSContext(newCtx, newGlobalObj, exception);
    if (exception && *exception)
      break;

    // sandbox
    JSObjectRef sandboxRef = JSValueToObject(ctx, arguments[1],  exception);
    if (exception && *exception)
      break;

    // clone sandbox ?
    if (false) {
      static JSStringRef cloneCodeStr = JSStringCreateWithUTF8CString("(function(){ return function(o){return Object.assign({},o);}})()");
      JSValueRef cloneCodeV = JSEvaluateScript(newCtx, cloneCodeStr, newGlobalObj, nullptr, 0, exception);
      if (exception && *exception)
        break;
      JSObjectRef coneFun = JSValueToObject(newCtx, cloneCodeV, exception);
      if (exception && *exception)
        break;
      JSValueRef args[] = { sandboxRef };
      JSValueRef resultRef = JSObjectCallAsFunction(newCtx, coneFun, newGlobalObj, 1, args, exception);
      if (exception && *exception)
        break;
      sandboxRef = JSValueToObject(newCtx, resultRef, exception);
      if (exception && *exception)
        break;
    }

    // setup global
    JSPropertyNameArrayRef namesRef = JSObjectCopyPropertyNames(newCtx, sandboxRef);
    size_t size = JSPropertyNameArrayGetCount(namesRef);
    for (size_t i = 0; i < size; ++i) {
      JSStringRef namePtr = JSPropertyNameArrayGetNameAtIndex(namesRef, i);
      JSValueRef valueRef = JSObjectGetProperty(newCtx, sandboxRef, namePtr, exception);
      if (exception && *exception)
        break;
      JSObjectSetProperty(newCtx, newGlobalObj, namePtr, valueRef, kJSPropertyAttributeNone, exception);
      if (exception && *exception)
        break;
    }
    JSPropertyNameArrayRelease(namesRef);
    if (exception && *exception)
      break;

    // code
    JSStringRef codeStr = JSValueToStringCopy(ctx, arguments[0], exception);
    if (exception && *exception)
      break;

    JSStringRef fileNameStr = JSValueToStringCopy(ctx, arguments[5], exception);
    if (exception && *exception) {
      JSStringRelease(codeStr);
      break;
    }
    JSGlobalContextSetName(newCtx, fileNameStr);

    JSValueRef evalResult = JSEvaluateScript(newCtx, codeStr, newGlobalObj, fileNameStr, 0, exception);
    JSStringRelease(codeStr);
    JSStringRelease(fileNameStr);
    if (exception && *exception)
      break;

    JSObjectRef funcObj = JSValueToObject(newCtx, evalResult, exception);
    if (exception && *exception)
      break;

    JSValueRef args[] = { arguments[3], arguments[4] };
    result = JSObjectCallAsFunction(newCtx, funcObj, newGlobalObj, 2, args, exception);
    if (exception && *exception)
      break;

    SandboxPrivate::init(ctx, sandboxRef, newCtx, newGlobalObj, exception);
  } while (0);

  JSGlobalContextRelease(newCtx);

  if (exception && *exception) {
    printException(ctx, *exception);
    return JSValueMakeUndefined(ctx);
  }

  return result;
}

void injectBindings(JSContextRef jsContext)
{
  auto injectFun =
      [](JSContextRef jsContext, const char* name, JSObjectCallAsFunctionCallback callback) {
          JSContextRef globalCtx = JSContextGetGlobalContext(jsContext);
          JSObjectRef globalObj = JSContextGetGlobalObject(globalCtx);
          JSStringRef funcName = JSStringCreateWithUTF8CString(name);
          JSObjectRef funcObj = JSObjectMakeFunctionWithCallback(jsContext, funcName, callback);
          JSObjectSetProperty(jsContext, globalObj, funcName, funcObj, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, nullptr);
          JSStringRelease(funcName);
      };

  injectFun(jsContext, "print", noopCallback);
  injectFun(jsContext, "require", requireCallback);
  injectFun(jsContext, "_exit", exitCallback);
  injectFun(jsContext, "_platform", noopCallback);
  injectFun(jsContext, "_hrtime", hrTimeCallback);
  injectFun(jsContext, "_readFile", readFileCallback);
  injectFun(jsContext, "_runInNewContext", runInNewContext);
  injectFun(jsContext, "_runInContext", runInContext);

  markJSContext(jsContext, nullptr, nullptr);
}

}  // namespace RtJSC
