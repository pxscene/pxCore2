// pxCore CopyRight 2007-2009 John Robinson
// Portable Framebuffer and Windowing Library
// pxOffscreenNative.h

#ifndef PX_OFFSCREEN_NATIVE_H
#define PX_OFFSCREEN_NATIVE_H

#define _WIN32_WINNT 0x400

#include <windows.h>

#include "../pxCore.h"
#include "../pxBuffer.h"

class pxOffscreenNative: public pxBuffer
{
public:
    pxOffscreenNative(): bitmap(NULL) {}

    HDC beginDrawingWithDC()
    {
        HDC dc = CreateCompatibleDC(NULL);
        savedBitmap = SelectObject(dc, bitmap);
        return dc;
    }

    void endDrawingWithDC(HDC dc)
    {
        SelectObject(dc, savedBitmap);
        DeleteDC(dc);
#ifndef WINCE
        GdiFlush();
#endif
    }

protected:

    HBITMAP bitmap;
    HGDIOBJ savedBitmap;
};

#endif