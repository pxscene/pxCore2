// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNative.cpp

#include "../pxCore.h"
#include "pxBufferNative.h"
#include "../pxRect.h"

#include "../pxOffscreen.h"

void pxBuffer::blit(pxSurfaceNative s, int dstLeft, int dstTop, int dstWidth, int dstHeight, 
    int srcLeft, int srcTop)
{
		Rect pr;
		MacSetRect(&pr, 0, 0, width(), height());	
		
		GWorldPtr gworld;
		NewGWorldFromPtr (&gworld, 32, &pr, NULL, NULL, 0, (char*)base(), 4*width());

		Rect dr, sr;
		MacSetRect(&dr, dstLeft, dstTop, dstLeft + dstWidth, dstTop + dstHeight);
		MacSetRect(&sr, srcLeft, srcTop, srcLeft + dstWidth, srcTop + dstHeight);
		CopyBits((BitMapPtr)*GetGWorldPixMap(gworld),(BitMapPtr)*GetGWorldPixMap(s),&sr,&dr,srcCopy,NULL);
	
		DisposeGWorld(gworld);
}
