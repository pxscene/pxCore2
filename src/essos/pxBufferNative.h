// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNative.h

#ifndef PX_BUFFER_NATIVE_H
#define PX_BUFFER_NATIVE_H

#include <wayland-client.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "../pxCore.h"

// Structure used to describe a window surface under X11
typedef struct
{
    uint32_t* pixelData;
    int windowWidth;
    int windowHeight;
} pxSurfaceNativeDesc;

typedef pxSurfaceNativeDesc* pxSurfaceNative;

#endif
