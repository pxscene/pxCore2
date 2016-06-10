// pxCore CopyRight 2007-2015 John Robinson
// pxScene2d.h

class testView: public pxIView
{
public:
  
testView(): mContainer(NULL),mw(0),mh(0),mEntered(false),mMouseX(0), mMouseY(0) {}
  virtual ~testView() {}

  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  virtual void RT_STDCALL onSize(int32_t w, int32_t h)
  {
    rtLogInfo("testView::onSize(%d, %d)", w, h);
    mw = w;
    mh = h;
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

  virtual bool RT_STDCALL onMouseEnter()
  {
    rtLogInfo("testView::onMouseEnter()");
    mEntered = true;
    return false;
  }

  virtual bool RT_STDCALL onMouseLeave()
  {
    rtLogInfo("testView::onMouseLeave()");
    mEntered = false;
    return false;
  }

  virtual bool RT_STDCALL onFocus()
  {
    rtLogInfo("testView::onFocus()");
    return false;
  }

  virtual bool RT_STDCALL onBlur()
  {
    rtLogInfo("testView::onBlur()");
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
};
