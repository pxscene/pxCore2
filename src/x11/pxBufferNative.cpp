// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNative.cpp

#include "../pxCore.h"
#include "pxBufferNative.h"
#include "../pxRect.h"

#include "../pxOffscreen.h"

void pxBuffer::blit(pxSurfaceNative s, int dstLeft, int dstTop, 
		    int dstWidth, int dstHeight, 
		    int srcLeft, int srcTop)
{
  XImage* image = ::XCreateImage(s->display, 
				 XDefaultVisual(s->display, 
						XDefaultScreen(s->display)), 
				 24,ZPixmap, 0, (char*)base(), 
				 width(), height(), 32, stride());
  
  if (image)
    {
      ::XPutImage(s->display, s->drawable, s->gc, image, srcLeft, srcTop, 
		  dstLeft, dstTop, dstWidth, dstHeight);
      
      // If we don't NULL this out XDestroyImage will damage
      // the heap by trying to free it internally
      image->data = NULL;
      XDestroyImage(image);
    }
}


