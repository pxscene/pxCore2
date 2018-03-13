// pxOffscreenNativeDfb.cpp

#include "../pxOffscreen.h"

#include <stdio.h>
#include <stdlib.h>

#include "pxBuffer.h"

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

// Assumes that SRC pix format is RGBA
//
void pxOffscreenNative::swizzleTo(rtPixelFmt fmt)
{
  // printf("\nDEBUG:   pxOffscreenNative::swizzleTo(rtPixelFmt fmt) - Format = %s (%d) ",
  //        rtPixelFmt2str(mPixelFormat), mPixelFormat); fflush(stdout); // JUNK
#if 1
  // Setup SRC indexes
  switch(mPixelFormat)
  {
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
      case RT_PIX_RGBA:
        mSrcIndexR = 0;
        mSrcIndexG = 1;
        mSrcIndexB = 2;
        mSrcIndexA = 3;
      //  printf("\nDEBUG:  swizzleTo() - SRC: RT_PIX_RGBA - %d%d%d%d",mSrcIndexR,mSrcIndexG,mSrcIndexB,mSrcIndexA );
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
      case RT_PIX_ARGB:
        mSrcIndexA = 3;
        mSrcIndexR = 2;
        mSrcIndexG = 1;
        mSrcIndexB = 0;
      //printf("\nDEBUG:  swizzleTo() - SRC: RT_PIX_ARGB - %d%d%d%d",mSrcIndexA,mSrcIndexR,mSrcIndexG,mSrcIndexB );
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
      case RT_PIX_BGRA:
        mSrcIndexB = 0;
        mSrcIndexG = 1;
        mSrcIndexR = 2;
        mSrcIndexA = 3;
      printf("\nDEBUG:  swizzleTo() - SRC: RT_PIX_BGRA ... not validated yet\n");
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
      case RT_PIX_RGB:
      printf("\nDEBUG:  swizzleTo() - SRC: RT_PIX_RGB ... not validated yet\n");
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
      case RT_PIX_A8:
      printf("\nDEBUG:  swizzleTo() - SRC: RT_PIX_A8 ... not validated yet\n");
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
  }//SWITCH

  // Setup DST indexes
  switch(fmt)
  {
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
      case RT_PIX_RGBA:
        mDstIndexR = 0;
        mDstIndexG = 1;
        mDstIndexB = 2;
        mDstIndexA = 3;
      //printf("\nDEBUG:  swizzleTo() - DST: RT_PIX_RGBA - %d%d%d%d  \n",mDstIndexR,mDstIndexG,mDstIndexB,mDstIndexA );
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
      case RT_PIX_ARGB:
        mDstIndexA = 3;
        mDstIndexR = 2;
        mDstIndexG = 1;
        mDstIndexB = 0;
     // printf("\nDEBUG:  swizzleTo() - DST: RT_PIX_ARGB - %d%d%d%d",mDstIndexA,mDstIndexR,mDstIndexG,mDstIndexB );
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
      case RT_PIX_BGRA:
        mDstIndexB = 3;
        mDstIndexG = 2;
        mDstIndexR = 1;
        mDstIndexA = 0;
      printf("\nDEBUG:  swizzleTo() - DST: RT_PIX_BGRA   ... not validated yet\n");
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
      case RT_PIX_RGB:
        mDstIndexR = 3;
        mDstIndexG = 2;
        mDstIndexB = 1;
      printf("\nDEBUG:  swizzleTo() - DST: RT_PIX_RGB   ... not validated yet\n");
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
      case RT_PIX_A8:
        mDstIndexR = 0;
        mDstIndexG = 0;
        mDstIndexB = 0;
        mDstIndexA = 0;
        printf("\nDEBUG:  swizzleTo() - DST: RT_PIX_A8   ... not validated yet\n");
      break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - -
  }//SWITCH

  uint8_t r = 0, g = 0, b = 0, a = 0;

//bool print = true;

  for (int y = 0; y < height(); y++)
  {
      pxPixel* p  = scanline(y);
      pxPixel* pe = p + width();

      while (p < pe)
      {
        // Copy SRC pixels
        r = p->bytes[mSrcIndexR];
        g = p->bytes[mSrcIndexG];
        b = p->bytes[mSrcIndexB];
        a = p->bytes[mSrcIndexA];

// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
// if(print)
// {
//   printf("\nDEBUG:  swizzleTo() - p->bytes[%d%d%d%d]: %02X %02X %02X %02X  \n",
//          mSrcIndexR,mSrcIndexG,mSrcIndexB,mSrcIndexA,
//          p->bytes[0], p->bytes[1], p->bytes[2], p->bytes[3]);
//   print = false;
// }
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG

        // Write DST pixels
        p->bytes[mDstIndexR] = r;
        p->bytes[mDstIndexG] = g;
        p->bytes[mDstIndexB] = b;
        p->bytes[mDstIndexA] = a;

        p++;
      }
  }


#else
  uint8_t r = 0, g = 0, b = 0, a = 0;

  for (int y = 0; y < height(); y++)
  {
      pxPixel* p  = scanline(y);
      pxPixel* pe = p + width();

      int x = 0;

      while (p < pe)
      {
        switch(mPixelFormat)  // source format
        {
          // - - - - - - - - - - - - - - - - - - - - - - - - - -
          case RT_PIX_RGBA:

            r = p->bytes[0]; // R
            g = p->bytes[1]; // G
            b = p->bytes[2]; // B
            a = p->bytes[3]; // A

      // if(y < 5 && x == 5)
      // {
      //   printf("\nBEFORE:  rgba: 0x%08X  r: 0x%02X  g: 0x%02X  b: 0x%02X  a: 0x%02X ",
      //     p->u, r, g, b, a);
      // }

            break;
          // - - - - - - - - - - - - - - - - - - - - - - - - - -
          case RT_PIX_ARGB:

            a = p->bytes[0]; // A
            r = p->bytes[1]; // R
            g = p->bytes[2]; // G
            b = p->bytes[3]; // B

      // if(y < 5 && x == 5)
      // {
      //   printf("\nBEFORE:  argb: 0x%08X  r: 0x%02X  g: 0x%02X  b: 0x%02X  a: 0x%02X ",
      //     p->u, r, g, b, a);
      // }
            break;
          // - - - - - - - - - - - - - - - - - - - - - - - - - -
          case RT_PIX_BGRA:

            b = p->bytes[3]; // B
            g = p->bytes[2]; // G
            r = p->bytes[1]; // R
            a = p->bytes[0]; // A

            break;

          // - - - - - - - - - - - - - - - - - - - - - - - - - -
          case RT_PIX_RGB:

            r = p->bytes[0]; // R
            g = p->bytes[1]; // G
            b = p->bytes[2]; // B
            a = 0;

            break;
          // - - - - - - - - - - - - - - - - - - - - - - - - - -
          case RT_PIX_A8:

            r = 0;
            g = 0;
            b = 0;
            a = p->bytes[0]; // A

            break;
          // - - - - - - - - - - - - - - - - - - - - - - - - - -
          default:
           printf("\nDEBUG:   pxOffscreenNative::swizzleTo() - mPixelFormat = UNKNOWN  ");
          // - - - - - - - - - - - - - - - - - - - - - - - - - -
        }//SWITCH

        // Swizzle bytes in place!
        p->a = a;
        p->r = r;
        p->g = g;
        p->b = b;

      // if(y < 5 && x == 5)
      // {
      //   printf("\n AFTER:  argb: 0x%08X  r: 0x%02X  g: 0x%02X  b: 0x%02X  a: 0x%02X ",
      //     p->u, p->r,p->g, p->b, p->a);
      // }

        p++;  x++;

      }//WHILE
  }//FOR
#endif

  mPixelFormat = RT_DEFAULT_PIX;
}
