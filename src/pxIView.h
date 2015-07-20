// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxIView.h

#ifndef PXIVIEW_H
#define PXIVIEW_H

#include "pxCore.h"
#include "pxRect.h"

#include "rtRefPtr.h"

// A pxIViewContainer must unregister itself
// upon being destroyed
class pxIViewContainer
{
public:    
  // In view coordinates on pixel boundaries
  // NULL means invalidate everything
  virtual void RT_STDCALL invalidateRect(pxRect* r) = 0;
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

  virtual void RT_STDCALL onMouseDown(int32_t x, int32_t y, uint32_t flags) = 0;
  virtual void RT_STDCALL onMouseUp(int32_t x, int32_t y, uint32_t flags) = 0;
  virtual void RT_STDCALL onMouseMove(int32_t x, int32_t y) = 0;

  virtual void RT_STDCALL onMouseEnter() = 0;
  virtual void RT_STDCALL onMouseLeave() = 0;

  virtual void RT_STDCALL onFocus() = 0;
  virtual void RT_STDCALL onBlur() = 0;

  virtual void RT_STDCALL onKeyDown(uint32_t keycode, uint32_t flags) = 0;
  virtual void RT_STDCALL onKeyUp(uint32_t keycode, uint32_t flags) = 0;
  virtual void RT_STDCALL onChar(uint32_t codepoint) = 0;

  virtual void RT_STDCALL onUpdate(double t) = 0;
  virtual void RT_STDCALL onDraw(/*pxBuffer& b, pxRect* r*/) = 0;

  virtual void RT_STDCALL setViewContainer(pxIViewContainer* l) = 0;

#if 0
  virtual rtError RT_STDCALL setURI(const char* s) = 0;
#endif
};

typedef rtRefPtr<pxIView> pxViewRef;

#if 0

rtError createView(const char* viewType, pxIView** view);

//typedef uint32_t (*fnGetKeyFlags)(int32_t wflags);
typedef rtError (*fnCreateView)(const char* viewType, pxIView** view);
#endif

#endif // PXIVIEW_H
