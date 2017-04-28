// pxCore CopyRight 2005-2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxOffscreenNative.cpp

#include "pxOffscreen.h"

pxError pxOffscreen::init(int width, int height)
{
	term();

  void* data = malloc(width*4*height);
  
  if (data)
  {
    setBase(data);
    setWidth(width);
    setHeight(height);
    setStride(width*4);
    setUpsideDown(false);
  }
  
  return data?PX_OK:PX_FAIL;
}

pxError pxOffscreen::term()
{
  void* data = base();
  if (data)
    free(data);
  setBase(NULL);
  setWidth(0);
  setHeight(0);
  setStride(0);
	return PX_OK;
}


