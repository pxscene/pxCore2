// pxCore CopyRight 2007-2009 John Robinson
// Portable Framebuffer and Windowing Library
// pxPixels.h

#ifndef PX_PIXELS_H
#define PX_PIXELS_H

#include "pxCore.h"
#include <inttypes.h>


#pragma pack(push, 4)

struct pxPixel {

    pxPixel() {}
    pxPixel(unsigned char _r, unsigned char _g, unsigned char _b)
    {
        r = _r;
        g = _g;
        b = _b;
        a = 255;
    }
    pxPixel(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a)
    {
        r = _r;
        g = _g;
        b = _b;
        a = _a;
    }
    pxPixel(uint32_t _u)
    {
        u = _u;
    }
	
	inline pxPixel(const pxPixel& p)
	{
		u = p.u;
	}

	pxPixel& operator=(const pxPixel& p)
	{
		u = p.u;
		return *this;
	}

    union 
    {
        struct 
        {
#ifdef PX_LITTLEENDIAN_PIXELS
            unsigned char b, g, r, a;
#else
            unsigned char a: 8;
            unsigned char r: 8;
            unsigned char g: 8;
            unsigned char b: 8;
#endif
        };
        uint32_t u;
    };
};

typedef pxPixel pxColor;

#pragma pack(pop)

#endif

