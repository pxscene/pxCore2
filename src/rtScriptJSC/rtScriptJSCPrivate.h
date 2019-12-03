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

#ifndef RTSCRIPTJSCPRIVATE_H
#define RTSCRIPTJSCPRIVATE_H

#include <JavaScriptCore/JavaScript.h>

#include <unordered_set>
#include <map>
#include <memory>

#include "rtJSCMisc.h"

// private
typedef const struct OpaqueJSWeak* JSWeakRef;

namespace RtJSC {

class rtJSCProtected;

class rtJSCContextPrivate
{
  friend class rtJSCProtected;

  int m_refCount { 0 };
  std::unordered_set<rtJSCProtected*> m_protected;
  std::map<rtString, std::unique_ptr<rtJSCProtected>> m_moduleCache;

  void addProtected(rtJSCProtected* o) {
    m_protected.insert(o);
  }

  void removeProtected(rtJSCProtected* o) {
    m_protected.erase(o);
  }
public:
  ~rtJSCContextPrivate();

  unsigned long AddRef() {
    return rtAtomicInc(&m_refCount);
  }

  unsigned long Release() {
    long l = rtAtomicDec(&m_refCount);
    if (l == 0) {
      delete this;
    }
    return l;
  }

  void releaseAllProtected();
  JSObjectRef findModule(const rtString &path);
  void addToModuleCache(const rtString &path, JSGlobalContextRef context, JSObjectRef module);

  static rtJSCContextPrivate* create(JSGlobalContextRef contextRef);
  static void setInCtx(JSGlobalContextRef contextRef, rtJSCContextPrivate*);
  static rtJSCContextPrivate* fromCtx(JSGlobalContextRef contextRef);
};

class rtJSCProtected final
{
  rtJSCProtected(const rtJSCProtected&) = delete;
  rtJSCProtected& operator=(const rtJSCProtected&) = delete;
  rtJSCProtected(rtJSCProtected&&) = delete;
  rtJSCProtected& operator=(rtJSCProtected&&) = delete;
protected:
  friend class rtJSCContextPrivate;

  JSGlobalContextRef m_contextRef { nullptr };
  JSObjectRef m_object { nullptr };
  rtJSCContextPrivate *m_priv { nullptr };

  void releaseProtected();
public:
  rtJSCProtected(JSGlobalContextRef context, JSObjectRef obj, rtJSCContextPrivate *priv);
  rtJSCProtected(JSContextRef context, JSObjectRef obj);
  virtual ~rtJSCProtected();

  JSObjectRef wrapped() const { return m_object; }
  JSGlobalContextRef context() const { return m_contextRef; }
};

class rtJSCWeak final
{
  JSContextGroupRef m_groupRef { nullptr };
  JSWeakRef m_weakRef { nullptr };

  void releaseWeakRef();
public:
  rtJSCWeak();
  rtJSCWeak(const rtJSCWeak& other);
  rtJSCWeak(rtJSCWeak&& other) noexcept;
  rtJSCWeak(JSContextRef context, JSObjectRef obj);
  ~rtJSCWeak();

  rtJSCWeak& operator=(const rtJSCWeak &);
  rtJSCWeak& operator=(rtJSCWeak &&) noexcept;

  JSObjectRef wrapped() const;
};

class rtJSCWrapperBase
{
protected:
  rtJSCProtected m_protectedObj;
public:
  rtJSCWrapperBase(JSContextRef context, JSObjectRef obj)
    : m_protectedObj(context, obj)
  { }
  virtual ~rtJSCWrapperBase()
  { }
  JSObjectRef wrapped() const { return m_protectedObj.wrapped(); }
  JSGlobalContextRef context() const { return m_protectedObj.context(); }
};

}  // RtJSC

#endif /* RTSCRIPTJSCPRIVATE_H */
