#ifndef PXIVIEW_H
#define PXIVIEW_H

#include "pxCore.h"
#include "pxRect.h"

//#include "rt.h"
#include "rtRefPtr.h"


// A pxIViewListener must unregister itself
// upon being destroyed
class pxIViewListener
{
public:    
    // In view coordinates on pixel boundaries
    virtual void __stdcall invalidateRect(pxRect* r) = 0;
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
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;

    // should make them __stdcall if I want it to be a binary
    // contract

    virtual void __stdcall onSize(int x, int y) = 0;

    virtual void __stdcall onMouseDown(int x, int y, unsigned long flags) = 0;
    virtual void __stdcall onMouseUp(int x, int y, unsigned long flags) = 0;
	virtual void __stdcall onMouseMove(int x, int y) = 0;
	virtual void __stdcall onMouseLeave() = 0;

    /* KEYS? */

    virtual void __stdcall onDraw(pxBuffer& b, pxRect* r) = 0;
   // virtual void __stdcall handleDraw(HDC dc, RECT* r) = 0;

    virtual void __stdcall addListener(pxIViewListener* listener) = 0;
    virtual void __stdcall removeListener(pxIViewListener* listener) = 0;
#if 0
    virtual rtError setBaseDirectory(const wchar_t* d) = 0;
    virtual rtError __stdcall setSrc(const wchar_t* s) = 0;
#endif
};

typedef rtRefPtr<pxIView> pxViewRef;

#if 0

rtError createView(int version, pxIView** view);

typedef unsigned (*fnGetKeyFlags)(int wflags);
typedef rtError (*fnCreateView)(int version, pxIView** view);
#endif

#endif // PXIVIEW_H