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

// pxWindow.h

#ifndef PX_WINDOW_H
#define PX_WINDOW_H

#include "pxOffscreen.h"
#include "pxCore.h"
#include "pxRect.h"

class pxWindow: public pxWindowNative
{
public:
  pxWindow() {}
  virtual ~pxWindow() {term();}
  
  // Windows are always created hidden
  // use setVisibility to show
  pxError init(int32_t left, int32_t top, int32_t width, int32_t height);
  
  pxError term(); // synonymous with close
  pxError close() { return term(); }
  
  bool visibility();
  void setVisibility(bool visible);
  
  virtual void RT_STDCALL invalidateRect(pxRect* r = NULL);
  
  void setTitle(const char* name);
  
  // This method enables the onAnimationTimer event
  // zero disables
  pxError setAnimationFPS(uint32_t fps);
  
  // obtain a pxSurfaceNative to perform platform native
  // drawing to a window outside of the onDraw event
  pxError beginNativeDrawing(pxSurfaceNative& s);
  pxError endNativeDrawing(pxSurfaceNative& s);
  
 protected:
  
  // Overrideable event methods
  virtual void onCreate() {}
  
  virtual void onCloseRequest() {}
  virtual void onClose() {}
  
  // To enable this event call setAnimationFPS defined above
  virtual void onAnimationTimer() {}
  
  virtual void onSize(int32_t /*w*/, int32_t /*h*/) {}
  
  // See constants used for flags below
  virtual void onMouseDown(int32_t /*x*/, int32_t /*y*/, uint32_t /*flags*/) {}
  virtual void onMouseUp(int32_t /*x*/, int32_t /*y*/, uint32_t /*flags*/) {}
  virtual void onMouseEnter() {}
  virtual void onMouseLeave() {}
  
  virtual void onFocus() {}
  virtual void onBlur() {}

  virtual void onMouseMove(int32_t /*x*/, int32_t /*y*/) {}
  virtual void onScrollWheel(float /*x*/, float /*y*/) {}
  
  // See pxWindowNative.h for keycode constants
  // See constants used for flags below
  virtual void onKeyDown(uint32_t /*keycode*/, uint32_t /*flags*/) {}
  virtual void onKeyUp(uint32_t /*keycode*/, uint32_t /*flags*/) {}
  virtual void onChar(uint32_t /*codepoint*/) {}
  
  // pxSurfaceNative abstracts a platform specific drawing surface
  // to perform platform specific drawing please see pxWindowNative.h
  // for the definition of this type
  virtual void onDraw(pxSurfaceNative /*s*/) {}
  
};

#endif

