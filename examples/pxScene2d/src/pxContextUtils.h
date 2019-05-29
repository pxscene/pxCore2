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

#ifndef PX_CONTEXT_UTILS_H
#define PX_CONTEXT_UTILS_H

#include "pxCore.h"
#include "rtRef.h"
#include "rtAtomic.h"

pxError deleteInternalGLContext(int id);
pxError createInternalContext(int &id);
pxError makeInternalGLContextCurrent(bool current, int id = 0);


class pxSharedContext
{
public:
    pxSharedContext() : mRef(0), mContextId(0), mIsCurrent(false)
    { }
    virtual ~pxSharedContext()
    {
      if (mContextId != 0)
      {
        if (mIsCurrent)
        {
          makeInternalGLContextCurrent(false, mContextId);
        }
        deleteInternalGLContext(mContextId);
      }
      mContextId = 0;
    }

    virtual unsigned long AddRef()
    {
      return rtAtomicInc(&mRef);
    }

    virtual unsigned long Release()
    {
      unsigned long l = rtAtomicDec(&mRef);
      if (l == 0)
        delete this;
      return l;
    }

    pxError makeCurrent(bool current)
    {
      if (mContextId == 0)
      {
        createInternalContext(mContextId);
      }
      mIsCurrent = current;
      return makeInternalGLContextCurrent(current, mContextId);
    }

protected:
    rtAtomic mRef;
    int32_t mContextId;
    bool mIsCurrent;
};

typedef rtRef<pxSharedContext> pxSharedContextRef;

#endif //PX_CONTEXT_UTILS_H
