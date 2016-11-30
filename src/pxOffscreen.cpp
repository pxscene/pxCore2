// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxOffscreen.cpp

#include "pxCore.h"
#include "pxOffscreen.h"

pxOffscreen::pxOffscreen() : mCompressedImageData(NULL), mCompressedImageDataSize(0)
{
}

pxOffscreen::~pxOffscreen()
{
  term();
  if (mCompressedImageData != NULL)
  {
    delete [] mCompressedImageData;
    mCompressedImageData = NULL;
  }
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

pxError pxOffscreen::compressedImageData(char*& data, size_t& dataSize)
{
  if (mCompressedImageData == NULL)
  {
     data = NULL;
     dataSize = 0;
  }
  else
  {
    if (data != NULL)
    {
      delete [] data;
    }
    data = new char[mCompressedImageDataSize];
    dataSize = mCompressedImageDataSize;
    memcpy(data, mCompressedImageData, dataSize);
  }
  return PX_OK;
}

void pxOffscreen::setCompressedImageData(const char* data, const size_t dataSize)
{
  if (mCompressedImageData != NULL)
  {
    delete [] mCompressedImageData;
  }
  mCompressedImageData = new char[dataSize];
  mCompressedImageDataSize = dataSize;
  memcpy(mCompressedImageData, data, mCompressedImageDataSize);
}

pxError pxOffscreen::moveCompressedImageDataTo(char*& destData, size_t& destDataSize)
{
  if (destData != NULL)
  {
    delete [] destData;
  }
  destData = mCompressedImageData;
  destDataSize = mCompressedImageDataSize;
  mCompressedImageData = NULL;
  mCompressedImageDataSize = 0;
  return PX_OK;
}

pxError pxOffscreen::transferCompressedImageDataFrom(char*& srcData, size_t& srcDataSize)
{
  if (mCompressedImageData != NULL)
  {
    delete [] mCompressedImageData;
  }
  mCompressedImageData = srcData;
  mCompressedImageDataSize = srcDataSize;
  srcData = NULL;
  srcDataSize = 0;
  return PX_OK;
}

