#include "../pxOffscreen.h"

#include <stdio.h>
#include <stdlib.h>

pxError pxOffscreen::init(int width, int height)
{
  term();

  mData = (char*) new unsigned char[width * height * 4];

  setBase(mData);
  setWidth(width);
  setHeight(height);
  setStride(width * 4);
  setUpsideDown(false);

  return PX_OK;
}

pxError pxOffscreen::term()
{
  return pxOffscreenNative::term();
}

pxError pxOffscreenNative::term()
{
  delete [] mData;
  mData = NULL;
  return PX_OK;
}



