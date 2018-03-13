// pxOffscreenNative.h

#ifndef PX_OFFSCREEN_NATIVE_H
#define PX_OFFSCREEN_NATIVE_H

#include "../pxCore.h"
#include "../pxBuffer.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

class pxOffscreenNative: public pxBuffer
{
public:
    pxOffscreenNative(): image(NULL), data(NULL) {}
    virtual ~pxOffscreenNative() {}

    pxError term();

    void swizzleTo(rtPixelFmt /*fmt*/) {};

protected:
    XImage* image;
    char* data;
};

#endif
