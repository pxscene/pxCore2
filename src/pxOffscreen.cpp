/*

 pxCore Copyright 2005-2017 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

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

pxError pxOffscreen::transferCompressedDataFrom(pxOffscreen& offscreen)
{
  freeCompressedData();
  mCompressedData = offscreen.mCompressedData;
  mCompressedDataSize = offscreen.mCompressedDataSize;
  offscreen.mCompressedData = NULL;
  offscreen.mCompressedDataSize = 0;
  return PX_OK;
}

