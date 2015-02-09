// pxCore CopyRight 2007-2009 John Robinson
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

pxError pxOffscreen::initWithColor(int width, int height, const pxColor& color)
{
  pxError e = init(width, height);
  fill(color);
  return e;
}


