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

// pxBuffer.h

#ifndef PX_BUFFER_H
#define PX_BUFFER_H

#include "pxColor.h"
#include "pxRect.h"
#include "pxCore.h"

#include <string.h> // memcpy
#include <stdlib.h>


typedef uint32_t rtPixelFmt;

#define RT_PIX_RGBA  1
#define RT_PIX_ARGB  2
#define RT_PIX_BGRA  3
#define RT_PIX_RGB   4
#define RT_PIX_A8    5 // Alpha only

inline const char* rtPixelFmt2str(rtPixelFmt f)
{
  switch(f)
  {
    case RT_PIX_RGBA: return "RT_PIX_RGBA";
    case RT_PIX_ARGB: return "RT_PIX_ARGB";
    case RT_PIX_BGRA: return "RT_PIX_BGRA";
    case RT_PIX_RGB:  return "RT_PIX_RGB";
    case RT_PIX_A8:   return "RT_PIX_A8";
    default:          return "(unknown)";
  }
}

// This class is used to point to and describe a 32bpp framebuffer
// The memory for this framebuffer is allocated and managed external
// to this class.
//

class pxBuffer
{
public:

pxBuffer(): mPixelFormat(RT_DEFAULT_PIX), 
            mSrcIndexR(0), mSrcIndexG(0), mSrcIndexB(0), mSrcIndexA(0),
            mDstIndexR(0), mDstIndexG(0), mDstIndexB(0), mDstIndexA(0),
            mBase(NULL), mWidth(0), mHeight(0), mStride(0), mUpsideDown(false) {}

  void* base() const { return mBase; }
  void setBase(void* p) { mBase = p; }

  int32_t width()  const { return mWidth; }
  void setWidth(int32_t width) { mWidth = width; }
  
  int32_t height() const { return mHeight; }
  void setHeight(int32_t height) { mHeight = height; }
  
  int32_t stride() const { return mStride; }
  void setStride(int32_t stride) { mStride = stride; }
  
  bool upsideDown() const { return mUpsideDown; }
  void setUpsideDown(bool upsideDown) { mUpsideDown = upsideDown; }

  int32_t sizeInBytes() const { return mStride * mHeight; }

  inline uint32_t *scanlineInt32(uint32_t line) const
  {
    return (uint32_t*)((uint8_t*)mBase +
                       ((mUpsideDown?(mHeight-line-1):line) * mStride));
  }

  inline pxPixel *scanline(int32_t line) const
  {
    return (pxPixel*)((uint8_t*)mBase +
                      ((mUpsideDown?(mHeight-line-1):line) * mStride));
  }

  inline pxPixel *pixel(int32_t x, int32_t y) const
  {
    return scanline(y) + x;
  }

  pxRect size()
  {
    return pxRect(0, 0, width(), height());
  }

  pxRect bounds() const
  {
    return pxRect(0, 0, width(), height());
  }

  void fill(const pxRect& r, const pxColor& color)
  {
    // calc clip
    pxRect c = bounds();
    c.intersect(r);

    for (int32_t i = c.top(); i < c.bottom(); i++)
    {
      pxPixel *p = pixel(c.left(), i);
      pxPixel *pe = p + c.width();
      while (p < pe)
      {
        *p++ = color;
      }
    }
  }

  void fill(const pxColor& color)
  {
    for (int32_t i = 0; i < height(); i++)
    {
      pxPixel *p = scanline(i);
      for (int32_t j = 0; j < width(); j++)
      {
        *p++ = color;
      }
    }
  }

  void fillAlpha(uint8_t alpha)
  {
    for (int32_t i = 0; i < height(); i++)
    {
      pxPixel*p = scanline(i);
      for (int32_t j = 0; j < width(); j++)
      {
        p->a = alpha;
        p++;
      }
    }
  }

  void blit(pxSurfaceNative s, int32_t dstLeft, int32_t dstTop,
            int32_t dstWidth, int32_t dstHeight,
            int32_t srcLeft, int32_t srcTop);

  inline void blit(pxSurfaceNative s)
  {
    blit(s, 0, 0, width(), height(), 0, 0);
  }

  inline void blit(const pxBuffer& b, int32_t dstLeft, int32_t dstTop,
                   int32_t dstWidth, int32_t dstHeight,
                   int32_t srcLeft, int32_t srcTop) const
  {
    pxRect srcBounds = bounds();
    pxRect dstBounds = b.bounds();

    pxRect srcRect(srcLeft, srcTop, srcLeft+dstWidth, srcTop+dstHeight);
    pxRect dstRect(dstLeft, dstTop, dstLeft+dstWidth, dstTop+dstHeight);

    srcBounds.intersect(srcRect);
    dstBounds.intersect(dstRect);

    int32_t l = dstBounds.left() + (srcBounds.left() - srcLeft);
    int32_t t = dstBounds.top() + (srcBounds.top() - srcTop);
    int32_t w = pxMin<int>(srcBounds.width(), dstBounds.width());
    int32_t h = pxMin<int>(srcBounds.height(), dstBounds.height());

    for (int32_t y = 0; y < h; y++)
    {
      pxPixel *s = pixel(srcBounds.left(), y+srcBounds.top());
      pxPixel *se = s + w;
      pxPixel *d = b.pixel(l, y+t);
      while(s < se)
      {
        *d++ = *s++;
      }
    }
  }

  inline void blit(const pxBuffer& b) const
  {
    blit(b, 0, 0, width(), height(), 0, 0);
  }

  inline void blit(pxBuffer& b, int32_t x, int32_t y) const
  {
    blit(b, x, y, width(), height(), 0, 0);
  }

  operator bool ()
  {
    return (mBase != 0);
  }

  // TODO:  Needs work...
  inline void flipVertical()
  {
//    printf("\n ##### %s .. FLIPPING !!! \n\n", __PRETTY_FUNCTION__);

    unsigned int bytes = sizeInBytes();
    unsigned int lw    = stride();

    char *src  = (char *) base();
    char *dst  = (char *) (src + bytes - lw); // last line

    char *line = (char *) malloc( stride() ); // single line in bytes

    for(int j=0; j < height(); j++, dst -= lw, src += lw)
    {
      // Copy line
      memcpy(line, dst, lw); // save

      memcpy(dst, src,  lw);
      memcpy(src, line, lw);
    }

    free(line); //cleanup
  }

  virtual void swizzleTo(rtPixelFmt fmt) = 0;

  rtPixelFmt mPixelFormat;

  // Swizzle indexes
  uint8_t mSrcIndexR, mSrcIndexG, mSrcIndexB, mSrcIndexA; // SRC
  uint8_t mDstIndexR, mDstIndexG, mDstIndexB, mDstIndexA; // DST

protected:
  void* mBase;
  int32_t mWidth;
  int32_t mHeight;
  int32_t mStride;
  bool mUpsideDown;
};

#endif

