// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxBuffer.h

#ifndef PX_BUFFER_H
#define PX_BUFFER_H

#include "pxColor.h"
#include "pxRect.h"
#include "pxCore.h"

#include <stdlib.h>

// This class is used to point to and describe a 32bpp framebuffer
// The memory for this framebuffer is allocated and managed external
// to this class.
//

class pxBuffer
{
public:

pxBuffer(): mBase(NULL), mWidth(0), mHeight(0), mStride(0), mUpsideDown(false) {}

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

  inline pxPixel *pixel(int32_t x, int32_t y)
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

  inline void blit(pxBuffer& b, int32_t dstLeft, int32_t dstTop,
                   int32_t dstWidth, int32_t dstHeight,
                   int32_t srcLeft, int32_t srcTop)
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

  inline void blit(pxBuffer& b)
  {
    blit(b, 0, 0, width(), height(), 0, 0);
  }

  inline void blit(pxBuffer& b, int32_t x, int32_t y)
  {
    blit(b, x, y, width(), height(), 0, 0);
  }

  operator bool ()
  {
    return (mBase != 0);
  }

protected:
  void* mBase;
  int32_t mWidth;
  int32_t mHeight;
  int32_t mStride;
  bool mUpsideDown;
};

#endif

