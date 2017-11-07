/*

 pxCore Copyright 2005-2017 John Robinson

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

// pxContextFramebuffer.h

#ifndef PX_CONTEXT_FRAMEBUFFER_H
#define PX_CONTEXT_FRAMEBUFFER_H

#include "rtRef.h"
#include "rtAtomic.h"

#include "pxCore.h"
#include "pxTexture.h"

typedef struct _pxContextState
{
  _pxContextState() : matrix(), alpha(1.0){}
  void copy(_pxContextState source)
  {
    matrix = source.matrix;
    alpha = source.alpha;
  }
  pxMatrix4f matrix;
  float alpha;
} pxContextState;

class pxContextFramebuffer
{
public:
  pxContextFramebuffer() : mRef(0), m_framebufferTexture(), m_framebufferStateStack(), mDirtyRectanglesEnabled(false), mDirtyRectangle() {}
  pxContextFramebuffer(pxTextureRef texture) : m_framebufferTexture(texture), m_framebufferStateStack(), mDirtyRectanglesEnabled(false), mDirtyRectangle() {}
  virtual ~pxContextFramebuffer() {resetFbo();}

  virtual unsigned long AddRef(){ return rtAtomicInc(&mRef);}

  virtual unsigned long Release()
  {
    unsigned long l = rtAtomicDec(&mRef);
    if (l == 0) {
      delete this;
    }
    return l;
  }

  void resetFbo()
  {
    if (m_framebufferTexture.getPtr() != NULL)
    {
      m_framebufferTexture->deleteTexture();
      m_framebufferTexture = NULL;
    }
    m_framebufferStateStack.clear();
  }

  void setTexture(pxTextureRef texture)
  {
    m_framebufferTexture = texture;
    m_framebufferTexture->enablePremultipliedAlpha(true);
  }

  pxTextureRef getTexture()
  {
    return m_framebufferTexture;
  }

  int width()
  {
    if (m_framebufferTexture.getPtr() != NULL)
    {
      return m_framebufferTexture->width();
    }
    return 0;
  }

  int height()
  {
    if (m_framebufferTexture.getPtr() != NULL)
    {
      return m_framebufferTexture->height();
    }
    return 0;
  }

  pxError currentState(pxContextState& state)
  {
    if (!m_framebufferStateStack.empty())
    {
      pxContextState contextState = m_framebufferStateStack.back();
      state.copy(contextState);
      return PX_OK;
    }
    return PX_FAIL;
  }

  void pushState(pxContextState contextState)
  {
    m_framebufferStateStack.push_back(contextState);
  }

  pxError popState(pxContextState& state)
  {
    if (!m_framebufferStateStack.empty())
    {
      pxContextState contextState = m_framebufferStateStack.back();
      m_framebufferStateStack.pop_back();
      state.copy(contextState);
      return PX_OK;
    }
    return PX_FAIL;
  }

  void enableDirtyRectangles(bool enable)
  {
    mDirtyRectanglesEnabled = enable;
  }

  bool isDirtyRectanglesEnabled()
  {
    return mDirtyRectanglesEnabled;
  }

  void setDirtyRectangle(int left, int top, int right, int bottom)
  {
    mDirtyRectangle.set(left, top, right, bottom);
  }
  pxRect dirtyRectangle()
  {
    return mDirtyRectangle;
  }

protected:
  rtAtomic mRef;
  pxTextureRef m_framebufferTexture;
  std::vector<pxContextState> m_framebufferStateStack;
  bool mDirtyRectanglesEnabled;
  pxRect mDirtyRectangle;
};

typedef rtRef<pxContextFramebuffer> pxContextFramebufferRef;

#endif //PX_CONTEXT_FRAMEBUFFER_H
