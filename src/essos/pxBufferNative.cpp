// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNative.cpp

#include "../pxCore.h"
#include "pxBufferNative.h"
#include "../pxRect.h"

#include "../pxOffscreen.h"

#include <wayland-client.h>


static void renderPixelsToBuffer(uint32_t *bufferData, unsigned char* pixelData, int width, int height)
{
    memcpy(bufferData, pixelData, width*height*4);
}

void pxBuffer::blit(pxSurfaceNative s, int dstLeft, int dstTop, 
		    int dstWidth, int dstHeight, 
		    int srcLeft, int srcTop)
{
    renderPixelsToBuffer(s->pixelData, (unsigned char*)base(), width(), height());
}


