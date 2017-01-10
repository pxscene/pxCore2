// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNative.h

#ifndef PX_BUFFER_NATIVE_H
#define PX_BUFFER_NATIVE_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include "../pxCore.h"

// Structure used to describe a window surface under X11
typedef struct
{
    Display* display;
    Drawable drawable;
    GC gc;
    int windowWidth;
    int windowHeight;
}
pxSurfaceNativeDesc;

typedef pxSurfaceNativeDesc* pxSurfaceNative;

#endif
