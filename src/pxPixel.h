// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxPixel.h

#ifndef PX_PIXELS_H
#define PX_PIXELS_H

#include "pxCore.h"
#include <inttypes.h>

#pragma pack(push, 4)

struct pxPixel {

  inline pxPixel() {}
  inline pxPixel(uint32_t _u) {u=_u;}
	inline pxPixel(const pxPixel& p) { u = p.u; }
  
  inline pxPixel(uint8_t _r,uint8_t _g,uint8_t _b,uint8_t _a=255)     
  {
    r=_r; 
    g=_g; 
    b=_b; 
    a=_a;
  }

	pxPixel& operator=(const pxPixel& p) { u = p.u; return *this; }

  union 
  {
    struct 
    {
#ifdef PX_LITTLEENDIAN_PIXELS
#ifdef PX_LITTLEENDIAN_RGBA_PIXELS
      uint8_t r,g,b,a;
#else
      uint8_t b,g,r,a;
#endif
#else
      uint8_t b: 8;   // LSB
      uint8_t g: 8;
      uint8_t r: 8;
      uint8_t a: 8;   // MSB
#endif
    };
    uint32_t u;
    uint8_t  bytes[4];
  };//UNION
};

typedef pxPixel pxColor;

#pragma pack(pop)

#endif

