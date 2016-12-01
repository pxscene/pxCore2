// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxOffscreen.cpp

#include "pxCore.h"
#include "pxOffscreen.h"

pxOffscreen::pxOffscreen() : mCompressedData(NULL), mCompressedDataSize(0)
{
}

pxOffscreen::~pxOffscreen()
{
  term();
  freeCompressedData();
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

pxError pxOffscreen::compressedData(char*& data, size_t& dataSize)
{
  if (mCompressedData == NULL)
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
    data = new char[mCompressedDataSize];
    dataSize = mCompressedDataSize;
    memcpy(data, mCompressedData, dataSize);
  }
  return PX_OK;
}

pxError pxOffscreen::compressedDataWeakReference(char*& data, size_t& dataSize)
{
  data = mCompressedData;
  dataSize = mCompressedDataSize;
  return PX_OK;
}

void pxOffscreen::setCompressedData(const char* data, const size_t dataSize)
{
  freeCompressedData();
  mCompressedData = new char[dataSize];
  mCompressedDataSize = dataSize;
  memcpy(mCompressedData, data, mCompressedDataSize);
}

pxError pxOffscreen::freeCompressedData()
{
  if (mCompressedData != NULL)
  {
    delete [] mCompressedData;
    mCompressedData = NULL;
  }
  mCompressedDataSize = 0;
  return PX_OK;
}

pxError pxOffscreen::moveCompressedDataTo(char*& destData, size_t& destDataSize)
{
  if (destData != NULL)
  {
    delete [] destData;
  }
  destData = mCompressedData;
  destDataSize = mCompressedDataSize;
  mCompressedData = NULL;
  mCompressedDataSize = 0;
  return PX_OK;
}

pxError pxOffscreen::transferCompressedDataFrom(char*& srcData, size_t& srcDataSize)
{
  freeCompressedData();
  mCompressedData = srcData;
  mCompressedDataSize = srcDataSize;
  srcData = NULL;
  srcDataSize = 0;
  return PX_OK;
}

