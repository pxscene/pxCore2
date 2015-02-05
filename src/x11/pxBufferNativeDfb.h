// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNativeDfb.h

#ifndef PX_BUFFER_NATIVE_DFB_H
#define PX_BUFFER_NATIVE_DFB_H

#include <directfb.h>

#include "../pxCore.h"

// Structure used to describe a window surface under X11
typedef struct
{
  IDirectFB          *dfb;
  IDirectFBSurface   *surface;

  int windowWidth;
  int windowHeight;

} pxSurfaceNativeDesc;

typedef pxSurfaceNativeDesc* pxSurfaceNative;

#endif //PX_BUFFER_NATIVE_DFB_H
