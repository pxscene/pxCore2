#ifndef PX_OFFSCREEN_NATIVE_H
#define PX_OFFSCREEN_NATIVE_H

#include "../pxCore.h"
#include "../pxBuffer.h"

class pxOffscreenNative: public pxBuffer
{
public:
    pxOffscreenNative(): mData(NULL) {}
    virtual ~pxOffscreenNative() {}

    pxError term();

protected:
    char* mData;
};

#endif
