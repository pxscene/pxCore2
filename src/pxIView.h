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

// pxIView.h

#ifndef PXIVIEW_H
#define PXIVIEW_H

#include "pxCore.h"
#include "pxRect.h"

#include "rtCore.h"
#include "rtRef.h"

// A pxIViewContainer must unregister itself
// upon being destroyed
class pxIViewContainer
{
public:    
  // In view coordinates on pixel boundaries
  // NULL means invalidate everything
  virtual void RT_STDCALL invalidateRect(pxRect* r) = 0;
  virtual void* RT_STDCALL getInterface(const char* t) = 0;
};

// TODO no way to have a scene draw to an arbitrary rectangle
// with beginDrawing and endDrawing

class pxIView
{
public:
  virtual unsigned long RT_STDCALL AddRef() = 0;
  virtual unsigned long RT_STDCALL Release() = 0;

  // should make them RT_STDCALL if I want it to be a binary
  // contract

  virtual void RT_STDCALL onSize(int32_t x, int32_t y) = 0;

  // events return true if the event was consumed by the view
  virtual bool RT_STDCALL onMouseDown(int32_t x, int32_t y, uint32_t flags) = 0;
  virtual bool RT_STDCALL onMouseUp(int32_t x, int32_t y, uint32_t flags) = 0;
  virtual bool RT_STDCALL onMouseMove(int32_t x, int32_t y) = 0;
  
  virtual bool RT_STDCALL onScrollWheel(float dx, float dy) { UNUSED_PARAM(dx); UNUSED_PARAM(dy); return false; };

  virtual bool RT_STDCALL onMouseEnter() = 0;
  virtual bool RT_STDCALL onMouseLeave() = 0;

  virtual bool RT_STDCALL onFocus() = 0;
  virtual bool RT_STDCALL onBlur() = 0;

  virtual bool RT_STDCALL onKeyDown(uint32_t keycode, uint32_t flags) = 0;
  virtual bool RT_STDCALL onKeyUp(uint32_t keycode, uint32_t flags) = 0;
  virtual bool RT_STDCALL onChar(uint32_t codepoint) = 0;

  virtual void RT_STDCALL onUpdate(double t) = 0;
  virtual void RT_STDCALL onDraw(/*pxBuffer& b, pxRect* r*/) = 0;

  virtual void RT_STDCALL setViewContainer(pxIViewContainer* l) = 0;
  virtual void RT_STDCALL onCloseRequest() {};
#if 0
  virtual rtError RT_STDCALL setURI(const char* s) = 0;
#endif
};

typedef rtRef<pxIView> pxViewRef;

#if 0

rtError createView(const char* viewType, pxIView** view);

//typedef uint32_t (*fnGetKeyFlags)(int32_t wflags);
typedef rtError (*fnCreateView)(const char* viewType, pxIView** view);
#endif

#endif // PXIVIEW_H
