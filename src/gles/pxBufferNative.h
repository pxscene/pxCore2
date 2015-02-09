#ifndef PX_BUFFER_NATIVE_H
#define PX_BUFFER_NATIVE_H

#include <stdint.h>

struct pxSurfaceNativeDesc
{
    uint32_t* pixelData;
    int windowWidth;
    int windowHeight;
};

typedef pxSurfaceNativeDesc* pxSurfaceNative;

#endif
