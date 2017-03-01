// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNative.cpp

#include "../pxCore.h"
#include "pxBufferNativeDfb.h"
#include "../pxRect.h"

#include "../pxOffscreen.h"

extern bool needsFlip;

#if _DEBUG_______

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


void pxBuffer::clearSurface(pxSurfaceNative s, const pxColor &clr /*= pxClear*/)
{
  if(s && s->surface)
  {
    DFB_CHECK( s->surface->Clear( s->surface, clr.r, clr.g, clr.b, clr.a ) );
  }
  else
  {
     printf("\nERROR:  pxBufferNativeDfb::clearSurface()  - bad args <<<<<< ERROR");
  }
}

#if 0

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
  dsc.flags  = (DFBSurfaceDescriptionFlags) (DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT | DSDESC_CAPS);
  dsc.caps   = DSCAPS_VIDEOONLY;

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

  DFB_CHECK (s->surface->Blit(s->surface, s->, NULL, x, y));

  //DFB_CHECK (s->surface->StretchBlit(s->surface, image, &rcSrc, &rcDst));

  DFB_CHECK (s->surface->Flip (s->surface, NULL, DSFLIP_WAITFORSYNC));

  if(image)
  {
    image->Release(image);
  }
}



#else

void pxBuffer::blit(pxSurfaceNative s, int dstLeft, int dstTop,
                    int dstWidth, int dstHeight,
                    int srcLeft, int srcTop)
{
  if( (base() == NULL) || (s == NULL) || (s->dfb == NULL) )
  {
    printf("\nERROR:  pxBufferNativeDfb::blit()  - bad args <<<<<< ERROR");
    return;
  }


  static IDirectFBSurface   *image = NULL;
  static DFBSurfaceDescription dsc;

  if(    dsc.width  != width()
      || dsc.height != height() )
  {
    if(image)
    {
      printf("\nERROR:  pxBufferNativeDfb::blit()   >>> NEW FRAME !");

      image->Release(image);
      image = NULL;
    }
  }

  if(image == NULL) // HACK
  {
    dsc.width  = width();
    dsc.height = height();
    dsc.flags  = (DFBSurfaceDescriptionFlags) (DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT);

    dsc.pixelformat           = DSPF_ARGB;
    dsc.preallocated[0].data  = base();      // Buffer is your data
    dsc.preallocated[0].pitch = width()*4;
    dsc.preallocated[1].data  = NULL;
    dsc.preallocated[1].pitch = 0;

    DFB_CHECK (s->dfb->CreateSurface( s->dfb, &dsc, &image ));

    if(image == NULL)  printf("\nERROR:  pxBufferNativeDfb::CreateSurface()   >>> FAILED !");
  }

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

  if(image)
  {
    // printf("\n [%d] ############# pxBuffer::blit() - WxH   src: %dx%d   >>   dst: %dx%d",
    //      gFrameNumber,  width(), height(), dstWidth, dstHeight);

  DFB_CHECK (s->surface->Blit(s->surface, image, NULL, 0, 0));

  //  DFB_CHECK (s->surface->StretchBlit(s->surface, image, &rcSrc, &rcDst));

  // DFB_CHECK (s->surface->Flip(s->surface, NULL, DSFLIP_NONE)); //DSFLIP_WAITFORSYNC));
  }

//############################################################################################################
//############################################################################################################

  needsFlip = true;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // if(image)
  // {
  //   image->Release(image);
  // }
}

#endif
