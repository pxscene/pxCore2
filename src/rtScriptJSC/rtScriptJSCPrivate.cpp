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

#include "rtScriptJSCPrivate.h"

#include <algorithm>
#include <map>
#include <memory>
#include <cassert>

#ifdef __cplusplus
extern "C" {
#endif

JS_EXPORT JSWeakRef JSWeakCreate(JSContextGroupRef, JSObjectRef);
JS_EXPORT void JSWeakRetain(JSContextGroupRef, JSWeakRef);
JS_EXPORT void JSWeakRelease(JSContextGroupRef, JSWeakRef);
JS_EXPORT JSObjectRef JSWeakGetObject(JSWeakRef);

#ifdef __cplusplus
}
#endif

namespace {

std::map<JSGlobalContextRef, RtJSC::rtJSCContextPrivate*> gPrivCtxCache;

static void rtJSCContextPrivate_finalize(JSObjectRef obj)
{
  RtJSC::rtJSCContextPrivate *priv = (RtJSC::rtJSCContextPrivate *)JSObjectGetPrivate(obj);
  JSObjectSetPrivate(obj, nullptr);
  priv->Release();
}

static const JSClassDefinition rtJSCContextPrivate_class_def =
{
  0,                              // version
  kJSClassAttributeNone,          // attributes
  "__rtJSCContextPrivate__class", // className
  nullptr,                        // parentClass
  nullptr,                        // staticValues
  nullptr,                        // staticFunctions
  nullptr,                        // initialize
  rtJSCContextPrivate_finalize,   // finalize
  nullptr,                        // hasProperty
  nullptr,                        // getProperty
  nullptr,                        // setProperty
  nullptr,                        // deleteProperty
  nullptr,                        // getPropertyNames
  nullptr,                        // callAsFunction
  nullptr,                        // callAsConstructor
  nullptr,                        // hasInstance
  nullptr                         // convertToType
};

static JSStringRef jscContextPrivateName()
{
  static JSStringRef nameStr = JSStringCreateWithUTF8CString("__rt_ctx_priv_obj");
  return nameStr;
}

static JSClassRef jscContextPrivateClass()
{
  static JSClassRef sClassRef = JSClassCreate(&rtJSCContextPrivate_class_def);
  return sClassRef;
}

}  // namespace

namespace RtJSC {

rtJSCContextPrivate* rtJSCContextPrivate::create(JSGlobalContextRef contextRef)
{
  auto priv = std::make_unique<rtJSCContextPrivate>();
  priv->setInCtx(contextRef, priv.get());
  return priv.release();
}

rtJSCContextPrivate::~rtJSCContextPrivate()
{
  assert(m_protected.empty());
  assert(m_moduleCache.empty());

  for (auto i = gPrivCtxCache.begin(), last = gPrivCtxCache.end(); i != last; )
  {
    if (i->second == this)
    {
      i = gPrivCtxCache.erase(i);
    }
    else
    {
      ++i;
    }
  }
}

void rtJSCContextPrivate::setInCtx(JSGlobalContextRef contextRef, rtJSCContextPrivate* priv)
{
  // released in rtJSCContextPrivate_finalize
  priv->AddRef();

  JSValueRef exception = nullptr;
  JSObjectRef globalObj = JSContextGetGlobalObject(contextRef);
  JSStringRef privName = jscContextPrivateName();
  JSObjectRef privObj = JSObjectMake(contextRef, jscContextPrivateClass(), priv);

  JSObjectSetProperty(
    contextRef, globalObj, privName, privObj,
    kJSPropertyAttributeDontEnum | kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, &exception);

  if (exception)
    printException(contextRef, exception);

  gPrivCtxCache[contextRef] = priv;
}

rtJSCContextPrivate* rtJSCContextPrivate::fromCtx(JSGlobalContextRef contextRef)
{
  auto it = gPrivCtxCache.find(contextRef);
  if (it != gPrivCtxCache.end())
    return it->second;

  JSValueRef exception = nullptr;
  JSObjectRef globalObj = JSContextGetGlobalObject(contextRef);
  JSStringRef privName = jscContextPrivateName();
  JSValueRef valueRef = JSObjectGetProperty(contextRef, globalObj, privName, &exception);
  if (exception) {
    printException(contextRef, exception);
    return nullptr;
  }

  JSObjectRef objectRef = JSValueToObject(contextRef, valueRef, &exception);
  if (exception) {
    printException(contextRef, exception);
    return nullptr;
  }
  return (rtJSCContextPrivate*)JSObjectGetPrivate(objectRef);
}

JSObjectRef rtJSCContextPrivate::findModule(const rtString &path)
{
  auto it = m_moduleCache.find(path);
  if (it != m_moduleCache.end()) {
    if (JSObjectRef res = it->second->wrapped())
      return res;
    m_moduleCache.erase(it);
  }
  return nullptr;
}

void rtJSCContextPrivate::addToModuleCache(const rtString &path, JSGlobalContextRef context, JSObjectRef module)
{
  m_moduleCache[path] = std::make_unique<rtJSCWeak>(context, module);
}

void rtJSCContextPrivate::releaseAllProtected()
{
  auto protectedSet = std::move(m_protected);
  m_protected.clear();
  for(auto &p : protectedSet)
    p->releaseProtected();
  m_moduleCache.clear();
}

rtJSCProtected::rtJSCProtected(JSContextRef context, JSObjectRef object)
  : m_contextRef(JSContextGetGlobalContext(context))
  , m_object(object)
{
//  JSGlobalContextRetain(m_contextRef);
  m_priv = rtJSCContextPrivate::fromCtx(m_contextRef);
  if (m_priv) {
    JSValueProtect(m_contextRef, m_object);
    m_priv->addProtected(this);
  } else {
//    JSGlobalContextRelease(m_contextRef);
    m_object = nullptr;
    m_contextRef = nullptr;
  }
}

rtJSCProtected::~rtJSCProtected()
{
  releaseProtected();
}

void rtJSCProtected::releaseProtected()
{
  if (m_contextRef && m_object) {
    RtJSC::assertIsMainThread();
    m_priv->removeProtected(this);
    JSValueUnprotect(m_contextRef, m_object);
//    JSGlobalContextRelease(m_contextRef);
    m_object = nullptr;
    m_contextRef = nullptr;
    m_priv = nullptr;
  }
}

rtJSCWeak::rtJSCWeak()
{
}

rtJSCWeak::rtJSCWeak(JSContextRef context, JSObjectRef obj)
{
  m_groupRef = JSContextGetGroup(JSContextGetGlobalContext(context));
  m_weakRef = JSWeakCreate(m_groupRef, obj);
  JSContextGroupRetain(m_groupRef);
}

rtJSCWeak::rtJSCWeak(const rtJSCWeak& other)
  : m_groupRef(other.m_groupRef)
  , m_weakRef(other.m_weakRef)
{
  if (m_groupRef && m_weakRef) {
    JSWeakRetain(m_groupRef, m_weakRef);
    JSContextGroupRetain(m_groupRef);
  }
}

rtJSCWeak::rtJSCWeak(rtJSCWeak&& other) noexcept
  : m_groupRef(std::exchange(other.m_groupRef, nullptr))
  , m_weakRef(std::exchange(other.m_weakRef, nullptr))
{
}

rtJSCWeak::~rtJSCWeak()
{
  RtJSC::assertIsMainThread();
  releaseWeakRef();
}

void rtJSCWeak::releaseWeakRef()
{
  if (m_groupRef && m_weakRef) {
    JSWeakRelease(m_groupRef, m_weakRef);
    JSContextGroupRelease(m_groupRef);
    m_groupRef = nullptr;
    m_weakRef = nullptr;
  }
}

rtJSCWeak& rtJSCWeak::operator=(const rtJSCWeak &other)
{
  if (this != &other) {
    releaseWeakRef();
    m_groupRef = other.m_groupRef;
    m_weakRef = other.m_weakRef;
    if (m_groupRef && m_weakRef) {
      JSWeakRetain(m_groupRef, m_weakRef);
      JSContextGroupRetain(m_groupRef);
    }
  }
  return *this;
}

rtJSCWeak& rtJSCWeak::operator=(rtJSCWeak &&other) noexcept
{
  if (this != &other) {
    releaseWeakRef();
    m_groupRef = std::exchange(other.m_groupRef, nullptr);
    m_weakRef = std::exchange(other.m_weakRef, nullptr);
  }
  return *this;
}

JSObjectRef rtJSCWeak::wrapped() const
{
  if (m_weakRef)
    return JSWeakGetObject(m_weakRef);
  return nullptr;
}

}  // RtJSC
