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

// pxViewWindow.h

#ifndef PX_VIEWWINDOW_H
#define PX_VIEWWINDOW_H

#include "pxWindow.h"
#include "pxIView.h"

class pxViewWindow: public pxWindow, public pxIViewContainer
{
public:

  // pxViewRef is a smart ptr
  // that manages the refcount on a pxIView instance
  pxError view(pxViewRef& v);
  // Specifying NULL will release the view
  pxError setView(pxIView* v);

protected:

  virtual void onClose();

  virtual void RT_STDCALL invalidateRect(pxRect* r)
  {
    pxWindow::invalidateRect(r);
  }

  virtual void onAnimationTimer();

  // The following methods are delegated to the view
  virtual void onSize(int32_t w, int32_t h);

  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags);
  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags);
  virtual void onMouseLeave();
  virtual void onMouseMove(int32_t x, int32_t y);

  virtual void onScrollWheel(float dx, float dy);

  virtual void onFocus();
  virtual void onBlur();

  virtual void onKeyDown(uint32_t keycode, uint32_t flags);
  virtual void onKeyUp(uint32_t keycode, uint32_t flags);
  virtual void onChar(uint32_t codepoint);

  virtual void onDraw(pxSurfaceNative s);

private:
  pxViewRef mView;
//  pxOffscreen mViewOffscreen;
  uint32_t mWidth, mHeight;
};

#endif // PX_VIEWWINDOW_H

