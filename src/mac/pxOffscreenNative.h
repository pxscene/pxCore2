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
