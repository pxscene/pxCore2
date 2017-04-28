// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNative.cpp

#include "../pxCore.h"
#include "pxBufferNative.h"
#include "../pxRect.h"

#include "../pxOffscreen.h"

#include "../pxWindow.h"

void pxBuffer::blit(pxSurfaceNative s, int32_t dstLeft, int32_t dstTop, 
                    int32_t dstWidth, int32_t dstHeight,
                    int32_t srcLeft, int32_t srcTop)
{
  pxWindow* w = (pxWindow*)s;
  if (w)
    w->blit(*this, dstLeft, dstTop, dstWidth, dstHeight, srcLeft, srcTop);
}

