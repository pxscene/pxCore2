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

// pxScene2d.h

class testView: public pxIView
{
public:
  
testView(): mContainer(NULL),mRefCount(0),mw(0),mh(0),mEntered(false),mMouseX(0), mMouseY(0), mScrollDX(0.0), mScrollDY(0.0) {}
  virtual ~testView() {}

  virtual unsigned long AddRef() 
  {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() 
  {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  virtual void RT_STDCALL onSize(int32_t w, int32_t h)
  {
    rtLogInfo("testView::onSize(%d, %d)", w, h);
    mw = static_cast<float>(w);
    mh = static_cast<float>(h);
  }

  virtual bool RT_STDCALL onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    rtLogInfo("testView::onMouseDown(%d, %d, %u)", x, y, flags);
    return false;
  }

  virtual bool RT_STDCALL onMouseUp(int32_t x, int32_t y, uint32_t flags)
  {
    rtLogInfo("testView::onMouseUp(%d, %d, %u)", x, y, flags);
    return false;
  }

  virtual bool RT_STDCALL onMouseMove(int32_t x, int32_t y)
  {
    rtLogInfo("testView::onMouseMove(%d, %d)", x, y);
    mMouseX = x;
    mMouseY = y;
    return false;
  }

  virtual bool RT_STDCALL onScrollWheel(float dx, float dy)
  {
    rtLogInfo("testView::onScrollWheel(%f, %f)", dx, dy);
    mScrollDX = dx;
    mScrollDY = dy;
    return false;
  }

  virtual bool RT_STDCALL onMouseEnter()
  {
    rtLogInfo("testView::onMouseEnter()");
    mEntered = true;
#ifdef PX_DIRTY_RECTANGLES
    if (mContainer)
    {
      pxRect dirtyRect(0,0,mw,mh);
      mContainer->invalidateRect(&dirtyRect);
    }
#endif //PX_DIRTY_RECTANGLES
    return false;
  }

  virtual bool RT_STDCALL onMouseLeave()
  {
    rtLogInfo("testView::onMouseLeave()");
#ifdef PX_DIRTY_RECTANGLES
    if (mContainer)
    {
      pxRect dirtyRect(0,0,mw,mh);
      mContainer->invalidateRect(&dirtyRect);
    }
#endif //PX_DIRTY_RECTANGLES
    mEntered = false;
    return false;
  }

  virtual bool RT_STDCALL onFocus()
  {
    rtLogInfo("testView::onFocus()");
#ifdef PX_DIRTY_RECTANGLES
    if (mContainer)
    {
      pxRect dirtyRect(0,0,mw,mh);
      mContainer->invalidateRect(&dirtyRect);
    }
#endif //PX_DIRTY_RECTANGLES
    return false;
  }

  virtual bool RT_STDCALL onBlur()
  {
    rtLogInfo("testView::onBlur()");
#ifdef PX_DIRTY_RECTANGLES
    if (mContainer)
    {
      pxRect dirtyRect(0,0,mw,mh);
      mContainer->invalidateRect(&dirtyRect);
    }
#endif //PX_DIRTY_RECTANGLES
    return false;
  }

  virtual bool RT_STDCALL onKeyDown(uint32_t keycode, uint32_t flags)
  {
    rtLogInfo("testView::onKeyDown(%u, %u)", keycode, flags);
    return false;
  }

  virtual bool RT_STDCALL onKeyUp(uint32_t keycode, uint32_t flags)
  {
    rtLogInfo("testView::onKeyUp(%u, %u)", keycode, flags);
    return false;
  }

  virtual bool RT_STDCALL onChar(uint32_t codepoint)
  {
    rtLogInfo("testView::onChar(%u)", codepoint);
    return false;
  }
  
  virtual void RT_STDCALL setViewContainer(pxIViewContainer* l)
  {
    rtLogInfo("testView::setViewContainer(%p)", l);
    mContainer = l;
  }

  virtual void RT_STDCALL onCloseRequest()
  {
    rtLogInfo("testView::onCloseRequest()");
  }
  
  virtual void RT_STDCALL onUpdate(double t);
  virtual void RT_STDCALL onDraw();


#if 0
  virtual rtError RT_STDCALL setUrl(const char* s) = 0;
#endif

private:
  pxIViewContainer *mContainer;
  rtAtomic mRefCount;
  float mw, mh;
  bool mEntered;
  int32_t mMouseX, mMouseY;
  float mScrollDX, mScrollDY;
};
