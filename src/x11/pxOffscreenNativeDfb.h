// pxCore CopyRight 2005-2006 John Robinson
// Portable Framebuffer and Windowing Library
// pxOffscreenNativeDfb.h

#ifndef PX_OFFSCREEN_NATIVE_DFB_H
#define PX_OFFSCREEN_NATIVE_DFB_H

#include "../pxCore.h"
#include "../pxBuffer.h"

#include <directfb.h>

class pxOffscreenNative: public pxBuffer
{
public:
    pxOffscreenNative(): image(NULL), data(NULL) {}
    virtual ~pxOffscreenNative() {}

    void swizzleTo(rtPixelFmt fmt);

    pxError term();

protected:
    IDirectFBSurface   *image;
    char* data;
};

#endif //PX_OFFSCREEN_NATIVE_DFB_H
