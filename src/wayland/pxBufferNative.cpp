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
    uint32_t *pixel = (uint32_t *)bufferData;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            *pixel++ = (pixelData[3] << 24) + (pixelData[2] << 16) + (pixelData[1] << 8) + pixelData[0];
            pixelData += 4;
        }
    }
}

void pxBuffer::blit(pxSurfaceNative s, int dstLeft, int dstTop, 
		    int dstWidth, int dstHeight, 
		    int srcLeft, int srcTop)
{
    renderPixelsToBuffer(s->pixelData, (unsigned char*)base(), width(), height());
}


