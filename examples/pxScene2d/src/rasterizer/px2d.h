#ifndef PX_2D_H
#define PX_2D_H

#include <math.h>

typedef unsigned long uint32;
#include "pxCore.h"

//#include "pxPixels.h"


#ifdef PX_PLATFORM_MAC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#pragma clang diagnostic ignored "-Wconversion"
#endif



enum pxFillMode
{
  fillEvenOdd = 1,
  fillWinding = 2
};

inline double pxSin(double i)
{
  return sin(i);
}

inline double pxCos(double i)
{
  return cos(i);
}


// preserves alpha

#if 1
inline void pxLerp2(uint32_t a, uint32_t &d, const uint32_t s)
{
	uint32 dstrb = d      & 0xFF00FF;
	uint32 dstag = d >> 8 & 0xFF00FF;

	uint32 srcrb = s      & 0xFF00FF;
	uint32 srcag = s >> 8 & 0xFF00FF;

	uint32 drb = srcrb - dstrb;
	uint32 dag = srcag - dstag;

	drb *= a;  
	dag *= a;  
	drb >>= 8;
	dag >>= 8;

	const uint32_t rb  = (drb + dstrb)      & 0x00FF00FF;
	const uint32_t ag  = (dag + dstag) << 8 & 0xFF00FF00;

	d = rb | ag;
}
#else
// doesn't preserve alpha
inline void pxLerp2(uint32 a, uint32 &d, const uint32 s)
{
	//const uint32 a     = (s >> 24) + 1;

	const uint32 dstrb = d & 0xFF00FF;
	const uint32 dstg  = d & 0xFF00;

	const uint32 srcrb = s & 0xFF00FF;
	const uint32 srcg  = s & 0xFF00;

	uint32 drb = srcrb - dstrb;
	uint32 dg  =  srcg - dstg;

	drb *= a;
	dg  *= a;  
	drb >>= 8;
	dg  >>= 8;

	uint32 rb = (drb + dstrb) & 0xFF00FF;
	uint32 g  = (dg  + dstg) & 0xFF00;

	d = rb | g;
}
#endif

// uses src alpha - doesn't preserve alpha
inline void pxBlend(uint32 &d, const uint32 s)
{
	const uint32 a     = (s >> 24) + 1;

	const uint32 dstrb = d & 0xFF00FF;
	const uint32 dstg  = d & 0xFF00;

	const uint32 srcrb = s & 0xFF00FF;
	const uint32 srcg  = s & 0xFF00;

	uint32 drb = srcrb - dstrb;
	uint32 dg  =  srcg - dstg;

	drb *= a;
	dg  *= a;  
	drb >>= 8;
	dg  >>= 8;

	uint32 rb = (drb + dstrb) & 0xFF00FF;
	uint32 g  = (dg  + dstg) & 0xFF00;


	d = rb | g;
}

// uses dst alpha - preserves alpha
inline bool pxBlendBehind (  
  uint32& src1, uint32 dst1)
{
#define _agmask					0xFF00FF00
#define _rbmask					0x00FF00FF

  uint32 da = src1>>24;

  if (da >= 255) 
    return 1;

  uint32 sa = dst1>>24;
  
  register		uint32 alpha1	= 256-da;
	register		uint32 agd1		= (dst1&0xff00)>>8;
	register		uint32 rbd1		=  dst1&_rbmask;

	register		uint32 ags1		=  src1&_agmask;
	register		uint32 rbs1		=  src1&_rbmask;
  
#if 1
  rbd1 *= sa;
  rbd1 >>= 8;
  rbd1 &= 0xff00ff;

  agd1 *= sa;
  agd1 >>= 8;
  agd1 &= 0xff;
  agd1 |= (sa << 16);

  agd1 							= ags1 +  (agd1*alpha1);									
	rbd1							= rbs1 + ((rbd1*alpha1)>>8);

#else
  rbd1 *= alpha1;
  rbd1 >>= 8;
  rbd1 &= 0xff00ff;

  agd1 *= alpha1;
  agd1 >>= 8;
  agd1 &= 0xff;
  agd1 |= (alpha1 << 16);

  agd1 							= ags1 +  (agd1*sa);									
	rbd1							= rbs1 + ((rbd1*sa)>>8);

#endif

	
	src1 = (agd1&_agmask) | (rbd1&_rbmask);
  return 0;
}

// Assumes premultiplied
inline bool pxPreMultipliedBlendBehind (  
  uint32_t& src1, uint32_t dst1)
{
//#define _agmask					0xFF00FF00
//#define _rbmask					0x00FF00FF
  uint32 a = src1>>24;
#if 1
  if (a >= 255) 
    return 1;
#endif
	register		uint32_t alpha1	= 256-a;
	register		uint32_t agd1		= (dst1&_agmask)>>8;
	register		uint32_t rbd1		=  dst1&_rbmask;
	register		uint32_t ags1		=  src1&_agmask;
	register		uint32_t rbs1		=  src1&_rbmask;
	agd1 							= ags1 +  (agd1*alpha1);									
	rbd1							= rbs1 + ((rbd1*alpha1)>>8);
	
	src1 = (agd1&_agmask) | (rbd1&_rbmask);
  return 0;
}


inline pxPixel pxBlend4(const pxPixel& s1, const pxPixel& s2, 
                        const pxPixel& s3, const pxPixel& s4, 
                        const uint32_t xp, const uint32_t yp)
{
#define _rbmask2 0xFF00FF
//#define _agmask2 0xFF00
#define _agmask2 0xFF00FF00

    
  uint32 arb = s1.u & _rbmask2;
  uint32 brb = s2.u & _rbmask2;
  uint32 crb = s3.u & _rbmask2;
  uint32 drb = s4.u & _rbmask2;
    
  uint32 aag = s1.u & _agmask2;
  uint32 bag = s2.u & _agmask2;
  uint32 cag = s3.u & _agmask2;
  uint32 dag = s4.u & _agmask2;
	
  uint32 rbdx1 = (brb     ) - (arb     );
  uint32 rbdx2 = (drb     ) - (crb     );
  uint32 agdx1 = (bag >> 8) - (aag >> 8);
  uint32 agdx2 = (dag >> 8) - (cag >> 8);
    
  uint32 rb1 = (arb + ((rbdx1 * xp) >> 8)) & _rbmask2;
  uint32 rb2 = (crb + ((rbdx2 * xp) >> 8)) & _rbmask2;
  uint32 ag1 = (aag + ((agdx1 * xp)     )) & _agmask2;
  uint32 ag2 = (cag + ((agdx2 * xp)     )) & _agmask2;

  uint32 rbdy = (rb2     ) - (rb1     );
  uint32 agdy = (ag2 >> 8) - (ag1 >> 8);
    
  uint32 rb = (rb1 + ((rbdy * yp) >> 8)) & _rbmask2;
  uint32 ag = (ag1 + ((agdy * yp)     )) & _agmask2;
    
  return pxPixel(ag | rb);
}

#endif

#ifdef PX_PLATFORM_MAC
#pragma clang diagnostic pop
#endif



