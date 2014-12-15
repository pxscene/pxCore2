// pxCore CopyRight 2007-2009 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNative.cpp

#include "../pxCore.h"
#include "pxBufferNative.h"
#include "../pxRect.h"

void pxBuffer::blit(pxSurfaceNative s, int dstLeft, int dstTop, int dstWidth, int dstHeight, 
    int srcLeft, int srcTop)
{
    int h = (upsideDown()?1:-1) * height();
    BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), width(), h, 1, 32 } };  

#ifndef WINCE
    // This behaves strangely on Windows CE
    // Appears to scale the blit by a factor of 2 on
    // a PocketPC 2003 device.  Emulator is not affected.
    // Any explanations as to why would be appreciated.

    int t = upsideDown()?(height()-srcTop-dstHeight):(srcTop);

    ::SetDIBitsToDevice(s, dstLeft, dstTop, dstWidth, dstHeight, srcLeft, t, 
        0, height(), base(), &bi, DIB_RGB_COLORS);
#else
    ::StretchDIBits(s, dstLeft, dstTop, dstWidth, dstHeight, srcLeft, srcTop, dstWidth, dstHeight, base(), &bi, DIB_RGB_COLORS, SRCCOPY);
#endif
}
