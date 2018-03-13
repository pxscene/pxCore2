/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

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
