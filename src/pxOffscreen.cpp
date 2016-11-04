// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxOffscreen.cpp

#include "pxCore.h"
#include "pxOffscreen.h"

pxOffscreen::pxOffscreen() 
{
}

pxOffscreen::~pxOffscreen()
{
	term();
}

void pxOffscreen::swizzleTo(rtPixelFmt fmt)
{
    pxOffscreenNative::swizzleTo(fmt);
}

pxError pxOffscreen::initWithColor(int32_t width, int32_t height, const pxColor& color)
{
  pxError e = init(width, height);
  fill(color);
  return e;
}


