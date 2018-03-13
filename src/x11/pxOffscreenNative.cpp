// pxOffscreenNative.cpp

#include "../pxOffscreen.h"

#include <stdio.h>
#include <stdlib.h>

pxError pxOffscreen::init(int width, int height)
{
  term();

  pxError e = PX_FAIL;

  data = (char*) new unsigned char[width * height * 4];

  if (data)
  {
    setBase(data);
    setWidth(width);
    setHeight(height);
    setStride(width*4);
    setUpsideDown(false);
    e = PX_OK;
  }

  return e;
}

pxError pxOffscreen::term()
{
  return pxOffscreenNative::term();
}

pxError pxOffscreenNative::term()
{
  delete [] data;
  data = NULL;

  return PX_OK;
}



