// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxIView.h

#ifndef PXIVIEW_H
#define PXIVIEW_H

#include "pxCore.h"
#include "pxRect.h"

#include "rtRefPtr.h"

// A pxIViewListener must unregister itself
// upon being destroyed
class pxIViewListener
{
public:    
  // In view coordinates on pixel boundaries
  virtual void RT_STDCALL invalidateRect(pxRect* r) = 0;
#if 0
  //virtual void __stdcall setCapture(bool capture) = 0;
  //  Like to eliminate these
  // since they are platform specific
  virtual void __stdcall beginDrawing(HDC& dc) = 0;
  virtual void __stdcall endDrawing(HDC& dc) = 0;
#endif
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

  virtual void RT_STDCALL onSize(int x, int y) = 0;

  virtual void RT_STDCALL onMouseDown(int x, int y, unsigned long flags) = 0;
  virtual void RT_STDCALL onMouseUp(int x, int y, unsigned long flags) = 0;
  virtual void RT_STDCALL onMouseMove(int x, int y) = 0;
  virtual void RT_STDCALL onMouseLeave() = 0;

  /* KEYS? */

  virtual void RT_STDCALL onDraw(pxBuffer& b, pxRect* r) = 0;
  // virtual void RT_STDCALL handleDraw(HDC dc, RECT* r) = 0;

  virtual void RT_STDCALL addListener(pxIViewListener* listener) = 0;
  virtual void RT_STDCALL removeListener(pxIViewListener* listener) = 0;
#if 0
  virtual rtError setBaseDirectory(const wchar_t* d) = 0;
  virtual rtError RT_STDCALL setSrc(const wchar_t* s) = 0;
#endif
};

typedef rtRefPtr<pxIView> pxViewRef;

#if 0

rtError createView(int version, pxIView** view);

typedef unsigned (*fnGetKeyFlags)(int wflags);
typedef rtError (*fnCreateView)(int version, pxIView** view);
#endif

#endif // PXIVIEW_H
