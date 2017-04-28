// pxCore CopyRight 2005-2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxOffscreenNative.h

#ifndef PX_OFFSCREEN_NATIVE_H
#define PX_OFFSCREEN_NATIVE_H

#include "../pxBuffer.h"

class pxOffscreenNative: public pxBuffer
{
public:
	pxOffscreenNative() {};

    void swizzleTo(rtPixelFmt /*fmt*/) {};
};

#endif