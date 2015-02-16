// pxCore CopyRight 2007-2009 John Robinson
// Portable Framebuffer and Windowing Library
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
    pxError init(int left, int top, int width, int height);

    pxError term(); // synonymous with close
    pxError close() { return term(); }

    bool visibility();
    void setVisibility(bool visible);

    void PXCALL invalidateRect(pxRect* r = NULL);

    void setTitle(char* name);

    // This method enables the onAnimationTimer event
	// zero disables
    pxError setAnimationFPS(long fps);

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

    virtual void onSize(int /*w*/, int /*h*/) {}

    // See constants used for flags below
    virtual void onMouseDown(int /*x*/, int /*y*/, unsigned long /*flags*/) {}
    virtual void onMouseUp(int /*x*/, int /*y*/, unsigned long /*flags*/) {}
    virtual void onMouseLeave() {}

    virtual void onMouseMove(int /*x*/, int /*y*/) {}

	// See pxWindowNative.h for keycode constants
    // See constants used for flags below
    virtual void onKeyDown(int /*keycode*/, unsigned long /*flags*/) {}
    virtual void onKeyUp(int /*keycode*/, unsigned long /*flags*/) {}
    virtual void onChar(char /*character*/) {}

	// pxSurfaceNative abstracts a platform specific drawing surface
	// to perform platform specific drawing please see pxWindowNative.h
	// for the definition of this type
    virtual void onDraw(pxSurfaceNative /*s*/) {}

};

// flags used in onMouseDown and onMouseUp
#define PX_LEFTBUTTON       1
#define PX_MIDDLEBUTTON     2
#define PX_RIGHTBUTTON      4

// flags used in onMouseDown, onMouseUp, onKeyDown, onKeyUp
#define PX_MOD_SHIFT        8
#define PX_MOD_CONTROL      16
#define PX_MOD_ALT          32

#endif

