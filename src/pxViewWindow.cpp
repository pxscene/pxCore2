// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxViewWindow.cpp

#include "pxViewWindow.h"

//#include "rtLog.h"

pxError pxViewWindow::view(pxViewRef& v)
{
  v = mView;
  return PX_OK;
}

pxError pxViewWindow::setView(pxIView* v)
{
#if 1
  if (mView.get())
  {
    mView->setViewContainer(NULL);
    mView = NULL;
  }
#endif

  mView = v;

  if (v)
    v->setViewContainer(this); 

  return PX_OK;
}

void pxViewWindow::onClose()
{
  setView(NULL);
}

void pxViewWindow::onSize(int32_t w, int32_t h)
{
  if (mView)
    mView->onSize(w, h);
}

void pxViewWindow::onMouseDown(int32_t x, int32_t y, uint32_t flags)
{
  if (mView)
    mView->onMouseDown(x, y, flags);
}

void pxViewWindow::onMouseUp(int32_t x, int32_t y, uint32_t flags)
{
  if (mView)
    mView->onMouseUp(x, y, flags);
}

void pxViewWindow::onMouseLeave()
{
  if (mView)
    mView->onMouseLeave();
}

void pxViewWindow::onFocus()
{
  if (mView) {
    mView->onFocus();
  }
}
void pxViewWindow::onBlur()
{
  if (mView) {
    mView->onBlur();
  }
}

void pxViewWindow::onMouseMove(int32_t x, int32_t y)
{
  if (mView)
    mView->onMouseMove(x, y);
}

void pxViewWindow::onKeyDown(uint32_t keycode, uint32_t flags)
{
  if (mView) {
    mView->onKeyDown(keycode, flags);
  }
}

void pxViewWindow::onKeyUp(uint32_t keycode, uint32_t flags)
{
  if (mView)
    mView->onKeyUp(keycode, flags);
}

void pxViewWindow::onChar(uint32_t codepoint)
{
  if (mView)
    mView->onChar(codepoint);
}

void pxViewWindow::onDraw(pxSurfaceNative s)
{
  if (mView)
    mView->onDraw(/*s*/);
}

