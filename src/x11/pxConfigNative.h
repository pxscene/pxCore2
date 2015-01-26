// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxConfigNative.h

#ifndef PX_CONFIGNATIVE_H
#define PX_CONFIGNATIVE_H

#define PXCALL

#define PX_LITTLEENDIAN_PIXELS

#ifndef PX_NATIVE
#include "pxBufferNative.h"
#include "pxOffscreenNative.h"
#ifdef ENABLE_GLUT
#include "pxWindowNativeGlut.h"
#else
#include "pxWindowNative.h"
#endif
#endif

#endif

