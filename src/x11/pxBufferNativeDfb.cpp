// pxBufferNative.cpp

#include "../pxCore.h"
#include "pxBufferNativeDfb.h"
#include "../pxRect.h"

#include "../pxOffscreen.h"

#if _DEBUG

#warning "DFB_CHECK() debug code included..."

//---------------------------------------------------------------
#define DFB_CHECK(x...)                                   \
{                                                         \
  DFBResult err = x;                                      \
                                                          \
  if (err != DFB_OK)                                      \
  {                                                       \
    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );\
    DirectFBErrorFatal( #x, err );                        \
  }                                                       \
}

#define DFB_ERROR(err)                                    \
  fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__,     \
  DirectFBErrorString(err));
//---------------------------------------------------------------

#else

#define DFB_CHECK(x...)   x;
#define DFB_ERROR(err)    err;

#endif


void pxBuffer::blit(pxSurfaceNative s, int dstLeft, int dstTop,
                    int dstWidth, int dstHeight,
                    int srcLeft, int srcTop)
{  
  if(base() == NULL)
  {
    printf("\nERROR:  pxBufferNativeDfb::blit()  base() is NULL   <<<<<< ERROR");
    return;
  }

  if(s == NULL)
  {
    printf("\nERROR:  pxBufferNativeDfb::blit() 's is NULL   <<<<<< ERROR");
    return;
  }

  if(s->dfb == NULL)
  {
    printf("\nERROR:  pxBufferNativeDfb::blit()   s->dfb is NULL   <<<<<< ERROR");
    return;
  }

  DFBSurfaceDescription dsc;

  dsc.width  = width();
  dsc.height = height();
  dsc.flags  = (DFBSurfaceDescriptionFlags) (DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT);
  dsc.caps   = DSCAPS_NONE;

  dsc.pixelformat           = DSPF_ARGB;
  dsc.preallocated[0].data  = base();      // Buffer is your data
  dsc.preallocated[0].pitch = width()*4;
  dsc.preallocated[1].data  = NULL;
  dsc.preallocated[1].pitch = 0;

  IDirectFBSurface   *image;
  DFB_CHECK (s->dfb->CreateSurface( s->dfb, &dsc, &image ));

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  DFBRectangle rcSrc;

  rcSrc.x = srcLeft;
  rcSrc.y = srcTop;
  rcSrc.w = width();
  rcSrc.h = height();

  DFBRectangle rcDst;

  rcDst.x = dstLeft;
  rcDst.y = dstTop;
  rcDst.w = dstWidth;
  rcDst.h = dstHeight;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  //  DFB_CHECK (s->surface->Blit(s->surface, s->, NULL, x, y));

  DFB_CHECK (s->surface->StretchBlit(s->surface, image, &rcSrc, &rcDst));

  DFB_CHECK (s->surface->Flip (s->surface, NULL, DSFLIP_WAITFORSYNC));

  if(image)
  {
    image->Release(image);
  }
}


