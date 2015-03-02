// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxViewWindow.cpp

#include "pxViewWindow.h"

#include "rtLog.h"

pxError pxViewWindow::view(pxViewRef& v)
{
  v = mView;
  return PX_OK;
}

pxError pxViewWindow::setView(pxIView* v)
{
  if (mView)
  {
    mView->removeListener(this);
    mView = NULL;
  }

  mView = v;

  if (v)
    v->addListener(this); 


  return PX_OK;
}

void pxViewWindow::onClose()
{
  setView(NULL);
}


void pxViewWindow::invalidateRect(pxRect* r)
{
  if (mView)
  {
#if 0
    pxRect b = mViewOffscreen.bounds();

    if (r)
    {
      b.intersect(*r);
      mView->onDraw(mViewOffscreen, &b);
    }
    else
#endif
      mView->onDraw(mViewOffscreen, r);


    pxSurfaceNative s;
    beginNativeDrawing(s);
#if 1
    if (r)
    {
      //rtLog("invalidateRect %d %d \n", r->left(), r->top());
#if 1
      mViewOffscreen.blit(s, r->left(), r->top(),
                          r->right()-r->left(), r->bottom()-r->top(),
                          r->left(), r->top());
#else
      mViewOffscreen.blit(s);
#endif

#if 0
      if (GetKeyState(VK_SCROLL) & 1)
      {
        // Draw dirty rect
        HPEN pen = CreatePen(PS_SOLID,1,RGB(255, 0, 0));
        HGDIOBJ brush = GetStockObject(NULL_BRUSH);
        HGDIOBJ oldPen = SelectObject(s, pen);
        HGDIOBJ oldBrush = SelectObject(s, brush);
        Rectangle(s, r->left(), r->top(), r->right(), r->bottom());
        SelectObject(s, oldBrush);
        SelectObject(s, oldPen);
        DeleteObject(pen);
        DeleteObject(brush);
      }
#endif
    }
    else
#endif
      mViewOffscreen.blit(s);
    endNativeDrawing(s);
  }
}

void pxViewWindow::onSize(int w, int h)
{
  mViewOffscreen.init(w, h);
#if 0
  if (mView)
  {
    mView->onSize(w, h);
  }
#else
  invalidateRect(NULL);
#endif
}

void pxViewWindow::onMouseDown(int x, int y, unsigned long flags)
{
  if (mView)
  {
    mView->onMouseDown(x, y, flags);
  }
}

void pxViewWindow::onMouseUp(int x, int y, unsigned long flags)
{
  if (mView)
  {
    mView->onMouseUp(x, y, flags);
  }
}

void pxViewWindow::onMouseLeave()
{
  if (mView)
  {
    mView->onMouseLeave();
  }
}

void pxViewWindow::onMouseMove(int x, int y)
{
  if (mView)
  {
    mView->onMouseMove(x, y);
  }
}

#if 0
void pxViewWindow::onKeyDown(int keycode, unsigned long flags)
{
  if (mView)
  {
    mView->onKeyDown(keycode, flags);
  }
}

void pxViewWindow::onKeyUp(int keycode, unsigned long flags)
{
  if (mView)
  {
    mView->onKeyUp(keycode, flags);
  }
}
#endif

void pxViewWindow::onDraw(pxSurfaceNative s)
{
  if (mView)
  {
    mViewOffscreen.blit(s);
  }
}

