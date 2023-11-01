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

#ifndef PX_SHAREDCONTEXT_H
#define PX_SHAREDCONTEXT_H

#include "rtAtomic.h"
#include "rtRef.h"

#include "pxCore.h"

class pxSharedContext: public pxSharedContextNative {
public:
  pxSharedContext(bool depthBuffer): pxSharedContextNative(depthBuffer) {}
  virtual ~pxSharedContext() {}

  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRef);
  }

  virtual unsigned long Release() {
    unsigned long l = rtAtomicDec(&mRef);
    if (l == 0)
      delete this;
    return l;
  }

  void makeCurrent(bool f);

protected:
  rtAtomic mRef;
};

typedef rtRef<pxSharedContext> pxSharedContextRef;

#endif //PX_CONTEXT_UTILS_H
