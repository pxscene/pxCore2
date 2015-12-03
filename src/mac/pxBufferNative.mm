// pxCore CopyRight 2005-2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNative.cpp

#include "../pxCore.h"
#include "pxBufferNative.h"
#include "../pxRect.h"

#include "../pxOffscreen.h"

#import <Cocoa/Cocoa.h>

void pxBuffer::blit(pxSurfaceNative s, int dstLeft, int dstTop, int dstWidth, int dstHeight, 
    int srcLeft, int srcTop)
{
  
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef bitmapCtx = CGBitmapContextCreate(mBase, mWidth, mHeight, 8, mWidth*4, colorSpace, kCGImageAlphaNoneSkipLast);

  CGImageRef ir = CGBitmapContextCreateImage(bitmapCtx);
  CGRect imageRect = CGRectMake(dstLeft, dstTop, dstWidth, dstHeight);
  
  
  CGContextSaveGState((CGContextRef)s);
  CGContextTranslateCTM((CGContextRef)s, 0, mHeight);
  CGContextScaleCTM((CGContextRef)s, 1.0, -1.0);
  CGContextDrawImage((CGContextRef)s, imageRect, ir);
  CGContextRestoreGState((CGContextRef)s);

  CGImageRelease(ir);
}
