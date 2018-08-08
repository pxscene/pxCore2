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

// pxViewWindow.cpp

#include "pxViewWindow.h"
#include "pxTimer.h"

//#include "rtLog.h"

pxError pxViewWindow::view(pxViewRef& v)
{
  v = mView;
  return PX_OK;
}

pxError pxViewWindow::setView(pxIView* v)
{
#if 1
  if (mView)
  {
    mView->setViewContainer(NULL);
    mView = NULL;
  }
#endif

  mView = v;

  if (v)
  {
    v->setViewContainer(this);
    v->onSize(mWidth,mHeight);
  }
    
  return PX_OK;
}

void pxViewWindow::onClose()
{
  setView(NULL);
}

void pxViewWindow::onSize(int32_t w, int32_t h)
{
  mWidth = w;
  mHeight = h;
  
  if (mView)
    mView->onSize(w, h);
}

void pxViewWindow::onScrollWheel(float dx, float dy)
{
  if (mView)
    mView->onScrollWheel(dx, dy);
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
  if (mView)
    mView->onKeyDown(keycode, flags);
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


void pxViewWindow::onAnimationTimer()
{
  if (mView)
  {
    mView->onUpdate(pxSeconds());
  }
}

void pxViewWindow::onDraw(pxSurfaceNative /*s*/)
{
  if (mView)
    mView->onDraw(/*s*/);
}

