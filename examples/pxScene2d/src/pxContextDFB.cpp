#include "rtDefs.h"
#include "rtLog.h"
#include "pxContext.h"

#include <directfb.h>


////////////////////////////////////////////////////////////////
//
// Debug macros...

// NOTE:  Comment out these defines for 'normal' operation.
//
// #define DEBUG_SKIP_RECT
// #define DEBUG_SKIP_IMAGE
// #define DEBUG_SKIP_IMAGE9

// #define DEBUG_SKIP_DIAG_RECT
// #define DEBUG_SKIP_DIAG_LINE

//#define DEBUG_SKIP_FLIPPING
//#define DEBUG_SKIP_BLIT

////////////////////////////////////////////////////////////////
//
// Debug Statistics
#ifdef USE_RENDER_STATS
  extern uint32_t gDrawCalls;
  extern uint32_t gTexBindCalls;
  extern uint32_t gFboBindCalls;

  #define TRACK_DRAW_CALLS()   { gDrawCalls++;    }
  #define TRACK_TEX_CALLS()    { gTexBindCalls++; }
  #define TRACK_FBO_CALLS()    { gFboBindCalls++; }
#else
  #define TRACK_DRAW_CALLS()
  #define TRACK_TEX_CALLS()
  #define TRACK_FBO_CALLS()
#endif

////////////////////////////////////////////////////////////////


#ifdef ENABLE_DFB_GENERIC
IDirectFB                *dfb = NULL;
IDirectFBSurface         *dfbSurface = NULL;
DFBSurfacePixelFormat     dfbPixelformat = DSPF_ABGR;
bool needsFlip = true;
IDirectFB                *outsideDfb = NULL;
IDirectFBSurface         *outsideDfbSurface = NULL;
pxContext context;
#else
extern IDirectFB                *dfb;

extern IDirectFBSurface         *dfbSurface;

extern DFBSurfacePixelFormat     dfbPixelformat;
extern bool needsFlip;
#endif //ENABLE_DFB_GENERIC

extern char* p2str(DFBSurfacePixelFormat fmt); // DEBUG


#if 0
// Use div0 to trigger debugger to break...

#define DFB_CHECK(x...)              \
{                                    \
   DFBResult err = x;                \
                                     \
   if (err != DFB_OK)                \
   {                                 \
      rtLogError( " DFB died... " ); \
      int n = 0;  int m = 2/n;\
         DirectFBErrorFatal( #x, err ); \
   }                                 \
}
#else

#define DFB_CHECK(x...)              \
{                                    \
   DFBResult err = x;                \
                                     \
   if (err != DFB_OK)                \
   {                                 \
      rtLogError( " DFB died... " ); \
      DirectFBErrorFatal( #x, err ); \
   }                                 \
}


#endif
//====================================================================================================================================================================================

//#warning TODO:  Use  pxContextSurfaceNativeDesc  within   pxFBOTexture ??

pxContextSurfaceNativeDesc  defaultContextSurface;
pxContextSurfaceNativeDesc* currentContextSurface = &defaultContextSurface;

pxContextFramebufferRef defaultFramebuffer(new pxContextFramebuffer());
pxContextFramebufferRef currentFramebuffer = defaultFramebuffer;

// TODO get rid of this global crap

static int gResW, gResH;
static pxMatrix4f gMatrix;
static float gAlpha = 1.0;

//====================================================================================================================================================================================

// more globals :(
static IDirectFBSurface  *boundTextureMask;
static IDirectFBSurface  *boundTexture;
static IDirectFBSurface  *boundFramebuffer = NULL; // default

void premultiply(float* d, const float* s)
{
   d[0] = s[0]*s[3];
   d[1] = s[1]*s[3];
   d[2] = s[2]*s[3];
   d[3] = s[3];
}

//====================================================================================================================================================================================

class pxFBOTexture : public pxTexture
{
public:
  pxFBOTexture() : mWidth(0), mHeight(0), mOffscreen(), mTexture(NULL)
  {
    mTextureType = PX_TEXTURE_FRAME_BUFFER;
  }

  ~pxFBOTexture()  { deleteTexture(); }

  void createTexture(int w, int h)
  {
    rtLogDebug("############# this: %p >>  %s  WxH: %d x %d \n", this, __PRETTY_FUNCTION__, w,h);

    if (mTexture !=NULL)
    {
        deleteTexture();
    }

    if(w <= 0 || h <= 0)
    {
        //rtLogDebug("\nERROR:  %s(%d, %d) - INVALID",__PRETTY_FUNCTION__, w,h);
        return;
    }

    rtLogDebug("############# this: %p >>  %s(%d, %d) \n", this, __PRETTY_FUNCTION__, w, h);

    mWidth  = w;
    mHeight = h;

    //boundTexture = mTexture; // it's an FBO

    mOffscreen.init(w,h);

    createSurface(mOffscreen); // surface is framebuffer

    DFB_CHECK(mTexture->Clear(mTexture, 0x00, 0x00, 0x00, 0x00 ) );  // TRANSPARENT
    //DFB_CHECK(mTexture->Clear(mTexture, 0x00, 0xFF, 0x00, 0xFF ) );  // DEBUG - GREEN !!!!

    //rtLogDebug("############# this: %p >>  %s - CLEAR_GREEN !!!!\n", this, __PRETTY_FUNCTION__);
  }

  pxError resizeTexture(int w, int h)
  {
    rtLogDebug("############# this: %p >>  %s  WxH: %d, %d\n", this, __PRETTY_FUNCTION__, w, h);

    if (mWidth != w || mHeight != h || !mTexture)
    {
        createTexture(w, h);
        return PX_OK;
    }

    // glActiveTexture(GL_TEXTURE3);
    // glBindTexture(GL_TEXTURE_2D, mTextureId);
    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
    //              width, height, GL_RGBA,
    //              GL_UNSIGNED_BYTE, NULL);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glUniform1f(u_alphatexture, 1.0);

    boundFramebuffer = mTexture;

    return PX_OK;
  }

  virtual pxError deleteTexture()
  {
    rtLogDebug("############# this: (virtual) %p >>  %s  ENTER\n",mTexture, __PRETTY_FUNCTION__);

    if(mTexture)
    {
        // if(currentFramebuffer == this)
        // {
        //   // Is this sufficient ? or Dangerous ?
        //   currentFramebuffer = NULL;
        //   boundTexture = NULL;
        // }

        mTexture->Release(mTexture);
        mTexture = NULL;
    }

    return PX_OK;
  }

  virtual pxError prepareForRendering()
  {
  //glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);
    boundFramebuffer = mTexture;

  //glBindTexture(GL_TEXTURE_2D, mTextureId);
  boundTexture = mTexture;

    rtLogDebug("############# this: (virtual) >>  %s  ENTER\n", __PRETTY_FUNCTION__);

    gResW = mWidth;
    gResH = mHeight;

    return PX_OK;
  }

// TODO get rid of pxError
// TODO get rid of bindTexture() and bindTextureAsMask()
  virtual pxError bindGLTexture(int tLoc)
  {
    (void) tLoc;

    if (mTexture == NULL)
    {
        rtLogDebug("############# this: %p >>  %s  ENTER\n", this,__PRETTY_FUNCTION__);
        return PX_NOTINITIALIZED;
    }

    boundTexture = mTexture;  TRACK_TEX_CALLS();

    return PX_OK;
  }

  virtual pxError bindGLTextureAsMask(int mLoc)
  {
    (void) mLoc;

    if (!mTexture)
    {
        rtLogDebug("############# this: %p >>  %s  ENTER\n", this,__PRETTY_FUNCTION__);
        return PX_NOTINITIALIZED;
    }

    boundTextureMask = mTexture;  TRACK_TEX_CALLS();

    return PX_OK;
  }

#if 1 // Do we need this?  maybe for some debugging use case??
  virtual pxError getOffscreen(pxOffscreen& o)
  {
    (void)o;
    // TODO
    return PX_FAIL;
  }
#endif

  virtual int width()   { return mWidth;  }
  virtual int height()  { return mHeight; }

//   void setWidth(int w)  { dsc.width  = mWidth  = w; }
//   void setHeight(int h) { dsc.height = mHeight = h; }

//   void                    setSurface(IDirectFBSurface* s)     { mTexture  = s; };

//   IDirectFBSurface*       getSurface()     { return mTexture; };
//   DFBVertex*              getVetricies()   { return &v[0];   };
//   DFBSurfaceDescription   getDescription() { return dsc;     };

  DFBVertex v[4];  //quad


private:
  int                     mWidth;
  int                     mHeight;

  pxOffscreen             mOffscreen;

  IDirectFBSurface       *mTexture;

  DFBSurfaceDescription   dsc;

private:

  pxError createSurface(pxOffscreen& o)
  {
    mWidth  = o.width();
    mHeight = o.height();

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    dsc.width                 = o.width();
    dsc.height                = o.height();
    dsc.flags                 = DFBSurfaceDescriptionFlags(DSDESC_WIDTH | DSDESC_HEIGHT| DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT);
    dsc.pixelformat           = dfbPixelformat;
    dsc.preallocated[0].data  = o.base();      // Buffer is your data
    dsc.preallocated[0].pitch = o.width() * 4;
    dsc.preallocated[1].data  = NULL;          // Not using a back buffer
    dsc.preallocated[1].pitch = 0;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    IDirectFBSurface  *tmp;

    DFB_CHECK(dfb->CreateSurface( dfb, &dsc, &tmp ) );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    dsc.width                 = o.width();
    dsc.height                = o.height();
    dsc.caps                  = DSCAPS_VIDEOONLY; // Use Video Memory //   DSCAPS_PREMULTIPLIED
    dsc.flags                 = DFBSurfaceDescriptionFlags(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT | DSDESC_CAPS);
    dsc.pixelformat           = dfbPixelformat;

    DFB_CHECK(dfb->CreateSurface( dfb, &dsc, &mTexture ) );

#ifndef DEBUG_SKIP_BLIT
    DFB_CHECK(mTexture->Blit(mTexture, tmp, NULL, 0,0)); TRACK_DRAW_CALLS(); // blit copy to surface
#endif

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    tmp->Release(tmp);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    return PX_OK;
  }
};// CLASS - pxFBOTexture

//====================================================================================================================================================================================

class pxTextureNone : public pxTexture
{
public:
  pxTextureNone() {}

  virtual int width()                                 { return 0; }
  virtual int height()                                { return 0; }
  virtual pxError deleteTexture()                     { return PX_FAIL; }
  virtual pxError resizeTexture(int /*w*/, int /*h*/) { return PX_FAIL; }
  virtual pxError getOffscreen(pxOffscreen& /*o*/)    { return PX_FAIL; }
  virtual pxError bindGLTexture(int /*tLoc*/)         { return PX_FAIL; }
  virtual pxError bindGLTextureAsMask(int /*mLoc*/)   { return PX_FAIL; }

};// CLASS - pxTextureNone

//====================================================================================================================================================================================

class pxTextureOffscreen : public pxTexture
{
public:
  pxTextureOffscreen() : mOffscreen(), mInitialized(false),
                        mTextureUploaded(false)
  {
    mTextureType = PX_TEXTURE_OFFSCREEN;
  }

  pxTextureOffscreen(pxOffscreen& o) : mOffscreen(), mInitialized(false),
                                      mTextureUploaded(false)
  {
    mTextureType = PX_TEXTURE_OFFSCREEN;

    createTexture(o);
  }

~pxTextureOffscreen() { deleteTexture(); };

  void createTexture(pxOffscreen& o)
  {
    mOffscreen.init(o.width(), o.height());

    // Flip the image data here so we match GL FBO layout
//      mOffscreen.setUpsideDown(true);

//#ifndef DEBUG_SKIP_BLIT
    o.blit(mOffscreen); TRACK_DRAW_CALLS();
//#endif

    // premultiply
    for (int y = 0; y < mOffscreen.height(); y++)
    {
        pxPixel* d = mOffscreen.scanline(y);
        pxPixel* de = d + mOffscreen.width();
        while (d < de)
        {
          d->r = (d->r * d->a)/255;
          d->g = (d->g * d->a)/255;
          d->b = (d->b * d->a)/255;
          d++;
        }
    }

    createSurface(o);

//JUNK
//     if(mTexture)
//     {
//        boundTexture = mTexture;

//        mTexture->Dump(mTexture,
//                      "/home/hfitzpatrick/projects/xre2/image_dumps",
//                      "image_");
//     }
//JUNK

    mInitialized = true;
  }

  virtual pxError deleteTexture()
  {
    rtLogInfo("pxTextureOffscreen::deleteTexture()");

    if(mTexture)
    {
        // if(currentFramebuffer == this)
        // {
        //   // Is this sufficient ? or Dangerous ?
        //   currentFramebuffer = NULL;
          boundTexture = NULL;
        // }

        mTexture->Release(mTexture);
        mTexture = NULL;
    }

    mInitialized = false;
    return PX_OK;
  }

  virtual pxError bindGLTexture(int tLoc)
  {
    (void) tLoc;

    if (!mInitialized)
    {
        return PX_NOTINITIALIZED;
    }

    if (!mTexture)
    {
        rtLogDebug("############# this: %p >>  %s  ENTER\n", this,__PRETTY_FUNCTION__);
        return PX_NOTINITIALIZED;
    }

    // TODO would be nice to do the upload in createTexture but right now it's getting called on wrong thread
    if (!mTextureUploaded)
    {
        createSurface(mOffscreen); // JUNK

  //     boundTexture = mTexture;

        mTextureUploaded = true;
    }
  //  else
    {
        boundTexture = mTexture;
    }

    return PX_OK;
  }

virtual pxError bindGLTextureAsMask(int mLoc)
  {
    (void) mLoc;

    if (!mInitialized)
    {
        return PX_NOTINITIALIZED;
    }

    if (!mTexture)
    {
        rtLogDebug("############# this: %p >>  %s  ENTER\n", this,__PRETTY_FUNCTION__);
        return PX_NOTINITIALIZED;
    }

    if (!mTextureUploaded)
    {
        createSurface(mOffscreen); // JUNK

  //     boundTextureMask = mTexture;

        mTextureUploaded = true;
    }
  //  else
    {
        boundTextureMask = mTexture;
    }

    return PX_OK;
  }

  virtual pxError getOffscreen(pxOffscreen& o)
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }

    o.init(mOffscreen.width(), mOffscreen.height());
//#ifndef DEBUG_SKIP_BLIT
    mOffscreen.blit(o);  TRACK_DRAW_CALLS();
//#endif

    return PX_OK;
  }

  virtual int width()  { return mOffscreen.width();  }
  virtual int height() { return mOffscreen.height(); }

private:
  pxOffscreen mOffscreen;
  bool        mInitialized;

  bool        mTextureUploaded;

  IDirectFBSurface       *mTexture;
  DFBSurfaceDescription   dsc;

private:

  pxError createSurface(pxOffscreen& o)
  {

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    dsc.width                 = o.width();
    dsc.height                = o.height();
    dsc.flags                 = DFBSurfaceDescriptionFlags(DSDESC_WIDTH | DSDESC_HEIGHT| DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT);
    dsc.pixelformat           = dfbPixelformat;
    dsc.preallocated[0].data  = o.base();      // Buffer is your data
    dsc.preallocated[0].pitch = o.width() * 4;
    dsc.preallocated[1].data  = NULL;          // Not using a back buffer
    dsc.preallocated[1].pitch = 0;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    IDirectFBSurface  *tmp;

    DFB_CHECK(dfb->CreateSurface( dfb, &dsc, &tmp ) );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    dsc.width                 = o.width();
    dsc.height                = o.height();
    dsc.caps                  = DSCAPS_VIDEOONLY;  // Use Video Memory  //   DSCAPS_PREMULTIPLIED
    dsc.flags                 = DFBSurfaceDescriptionFlags(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT | DSDESC_CAPS);
    dsc.pixelformat           = dfbPixelformat;

    DFB_CHECK(dfb->CreateSurface( dfb, &dsc, &mTexture ) );

#ifndef DEBUG_SKIP_BLIT
    DFB_CHECK(mTexture->Blit(mTexture, tmp, NULL, 0,0)); TRACK_DRAW_CALLS(); // blit copy to surface
#endif

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    tmp->Release(tmp);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    return PX_OK;
  }

}; // CLASS - pxTextureOffscreen

//====================================================================================================================================================================================

class pxTextureAlpha : public pxTexture
{
public:
  pxTextureAlpha() :  mDrawWidth(0.0),  mDrawHeight(0.0),
                    mImageWidth(0.0), mImageHeight(0.0),
                    mTexture(NULL), mBuffer(NULL), mInitialized(false)
  {
    mTextureType = PX_TEXTURE_ALPHA;
  }

  pxTextureAlpha(float w, float h, float iw, float ih, void* buffer)
    :  mDrawWidth(w),   mDrawHeight(h),
      mImageWidth(iw), mImageHeight(ih),
      mTexture(NULL), mBuffer(NULL), mInitialized(false)
  {
    mTextureType = PX_TEXTURE_ALPHA;

    // copy the pixels
    int bitmapSize = ih * iw;
    mBuffer = malloc(bitmapSize);

#if 1
    memcpy(mBuffer, buffer, bitmapSize);
#else
    // TODO consider iw,ih as ints rather than floats...
    int32_t bw = (int32_t)iw;
    int32_t bh = (int32_t)ih;

    // Flip here so that we match FBO layout...
    for (int32_t i = 0; i < bh; i++)
    {
      uint8_t *s = (uint8_t*)buffer+(bw*i);
      uint8_t *d = (uint8_t*)mBuffer+(bw*(bh-i-1));
      uint8_t *de = d+bw;
      while(d<de)
        *d++ = *s++;
    }
#endif

      // TODO Moved this to bindTexture because of more pain from JS thread calls
//      createTexture(w, h, iw, ih);

//      //JUNK
//       if(mTexture)
//       {
//          boundTexture = mTexture;

//          mTexture->Dump(mTexture,
//                        "/home/hfitzpatrick/projects/xre2/image_dumps",
//                        "image_alpha_");
//       }
//      //JUNK
   }

   ~pxTextureAlpha()
   {
      if(mBuffer)
      {
         free(mBuffer);
      }
      deleteTexture();
   }

  void createTexture(float w, float h, float iw, float ih)
  {
    rtLogDebug("############# this: %p >>  %s  WxH: %.0f x %.0f \n", this, __PRETTY_FUNCTION__, w,h);

    if (mTexture != NULL)
    {
        deleteTexture();
    }

    if(iw == 0 || ih == 0)
    {
        return;
    }

    mDrawWidth   = w;
    mDrawHeight  = h;

    mImageWidth  = iw;
    mImageHeight = ih;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Load DFB surface with TEXTURE

    dsc.width                 = iw;
    dsc.height                = ih;
#if 1
    dsc.flags                 = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT);// | DSDESC_CAPS);
    //      dsc.caps                  = DSCAPS_VIDEOONLY;  // GFX Memory  DSCAPS_VIDEOONLY;// DSCAPS_SYSTEMONLY; //DSCAPS_NONE;  //DSCAPS_VIDEOONLY
#else
    dsc.flags                 = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT | DSDESC_CAPS);
    dsc.caps                  = DSCAPS_VIDEOONLY;//DSCAPS_VIDEOONLY;// DSCAPS_SYSTEMONLY; //DSCAPS_NONE;  //DSCAPS_VIDEOONLY
#endif

    dsc.pixelformat           = DSPF_A8;      // ALPHA !!
    dsc.preallocated[0].data  = mBuffer;      // Buffer is your data
    dsc.preallocated[0].pitch = iw;
    dsc.preallocated[1].data  = NULL;         // Not using a back buffer
    dsc.preallocated[1].pitch = 0;

    DFB_CHECK (dfb->CreateSurface( dfb, &dsc, &mTexture));

    boundTexture = mTexture;

    mInitialized = true;
  }

  virtual pxError deleteTexture()
  {
    rtLogDebug("############# this: (virtual) %p >>  %s  ENTER\n",mTexture, __PRETTY_FUNCTION__);

    if (mTexture != NULL)
    {
        mTexture->Release(mTexture);
        mTexture = NULL;
    }

    mInitialized = false;
    return PX_OK;
  }

  virtual pxError bindGLTexture(int tLoc)
  {
    (void) tLoc;

    // TODO Moved to here because of js threading issues
    if (!mInitialized) createTexture(mDrawWidth,mDrawHeight,mImageWidth,mImageHeight);
    if (!mInitialized)
    {
        //rtLogDebug("############# this: %p >>  %s  PX_NOTINITIALIZED\n", this, __PRETTY_FUNCTION__);

        return PX_NOTINITIALIZED;
    }

    boundTexture = mTexture;

    return PX_OK;
  }

  virtual pxError bindGLTextureAsMask(int mLoc)
  {
    (void) mLoc;

    if (!mInitialized)
    {
        rtLogDebug("############# this: %p >>  %s  PX_NOTINITIALIZED\n", this, __PRETTY_FUNCTION__);

        return PX_NOTINITIALIZED;
    }

    boundTextureMask = mTexture;

    return PX_OK;
  }

  virtual pxError getOffscreen(pxOffscreen& /*o*/)
  {
    rtLogDebug("############# this: %p >>  %s  ENTER\n", this, __PRETTY_FUNCTION__);

    if (!mInitialized)
    {
        return PX_NOTINITIALIZED;
    }
    return PX_FAIL;
  }

  virtual int width()  { return mDrawWidth;  }
  virtual int height() { return mDrawHeight; }

  //DFBVertex*              getVetricies()   { return &v[0];   };
  //DFBVertex v[4];  //quad

  IDirectFBSurface*       getSurface()     { return mTexture; };
  DFBSurfaceDescription   getDescription() { return dsc;     };

private:
  float mDrawWidth;
  float mDrawHeight;

  float mImageWidth;
  float mImageHeight;

  IDirectFBSurface       *mTexture;
  DFBSurfaceDescription   dsc;

  void                   *mBuffer;
  bool                    mInitialized;

}; // CLASS - pxTextureAlpha

//====================================================================================================================================================================================

// SOLID
void draw_SOLID(int resW, int resH, float* matrix, float alpha,
                DFBRectangle &src, DFBRectangle &dst,
                pxTextureRef texture, const float* color)
{
  (void) resW; (void) resH; (void) matrix; (void) alpha; (void) src;

  float colorPM[4];
  premultiply(colorPM, color);

  texture->bindGLTexture(0);

  DFB_CHECK(boundFramebuffer->SetBlittingFlags(boundFramebuffer, (DFBSurfaceBlittingFlags) (DSBLIT_COLORIZE | DSBLIT_BLEND_ALPHACHANNEL) ));

  DFB_CHECK(boundFramebuffer->SetColor( boundFramebuffer, colorPM[0] * 255.0, // RGBA
                                                          colorPM[1] * 255.0,
                                                          colorPM[2] * 255.0,
                                                          colorPM[3] * 255.0));

#ifndef DEBUG_SKIP_BLIT
  DFB_CHECK(boundFramebuffer->Blit(boundFramebuffer, boundTexture, NULL, dst.x , dst.y));
#endif

  DFB_CHECK(boundFramebuffer->SetBlittingFlags(boundFramebuffer, (DFBSurfaceBlittingFlags) (DSBLIT_BLEND_ALPHACHANNEL) ));
}

//====================================================================================================================================================================================

// TEXTURE
void draw_TEXTURE(int resW, int resH, float* matrix, float alpha,
                  DFBRectangle &src, DFBRectangle &dst,
                  pxTextureRef texture,
                  int32_t xStretch = 0, int32_t yStretch = 0)
{
  (void) resW; (void) resH; (void) matrix; (void) alpha;

  texture->bindGLTexture(0);

#ifndef DEBUG_SKIP_BLIT
  if(xStretch == pxConstantsStretch::REPEAT ||
     yStretch == pxConstantsStretch::REPEAT)
  {
    DFB_CHECK(boundFramebuffer->TileBlit(boundFramebuffer, boundTexture,  &src, dst.x , dst.y));
  }
  else
  if(xStretch == pxConstantsStretch::STRETCH ||
     yStretch == pxConstantsStretch::STRETCH)
  {
    DFB_CHECK(boundFramebuffer->StretchBlit(boundFramebuffer, boundTexture, &src, &dst));
  }
  else
  {
    DFB_CHECK(boundFramebuffer->Blit(boundFramebuffer, boundTexture, NULL, dst.x , dst.y));
  }
#endif // DEBUG_SKIP_BLIT
}

//====================================================================================================================================================================================

// MASK
void draw_MASK(int resW, int resH, float* matrix, float alpha,
               DFBRectangle &src, DFBRectangle &dst,
               pxTextureRef texture,
               pxTextureRef mask)
{
  (void) resW; (void) resH; (void) matrix; (void) alpha; (void) src;

  printf("\n >>>> Draw MASK");

  texture->bindGLTexture(0);

  mask->bindGLTextureAsMask(0);

  DFB_CHECK(boundFramebuffer->SetBlittingFlags(boundFramebuffer, (DFBSurfaceBlittingFlags) (DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_BLEND_COLORALPHA) ));

#ifndef DEBUG_SKIP_BLIT
  DFB_CHECK(boundFramebuffer->Blit(boundFramebuffer, boundTexture, NULL, dst.x , dst.y));
#endif

  DFB_CHECK(boundFramebuffer->SetBlittingFlags(boundFramebuffer, (DFBSurfaceBlittingFlags) (DSBLIT_BLEND_ALPHACHANNEL) ));
}


//====================================================================================================================================================================================

static void drawRect2(float x, float y, float w, float h, const float* c)
{
  if( boundFramebuffer == 0)
  {
    rtLogError("cannot drawRect2() on context surface because surface is NULL");
    return;
  }

  const float verts[4][2] =
  {
    {  x,   y },
    {  x+w, y },
    {  x,   y+h },
    {  x+w, y+h }
  };

  (void) verts; // WARNING

  float colorPM[4];
  premultiply(colorPM,c);

  DFB_CHECK( boundFramebuffer->SetColor( boundFramebuffer, colorPM[0] * 255.0, // RGBA
                                                           colorPM[1] * 255.0,
                                                           colorPM[2] * 255.0,
                                                           colorPM[3] * 255.0));

  // DFB_CHECK( boundFramebuffer->FillRectangle( boundFramebuffer, x, y, w, h));  // HACK
}

static void drawRectOutline(float x, float y, float w, float h, float lw, const float* c)
{
  if( boundFramebuffer == 0)
  {
    rtLogError("cannot drawRectOutline() on context surface because surface is NULL");
    return;
  }

  float colorPM[4];
  premultiply(colorPM, c);

  DFB_CHECK( boundFramebuffer->SetColor(  boundFramebuffer, colorPM[0] * 255.0, // RGBA
                                                            colorPM[1] * 255.0,
                                                            colorPM[2] * 255.0,
                                                            colorPM[3] * 255.0));
  if(lw >= w)
  {
    lw = 0;
  }

  float half =  1; // lw/2

  DFB_CHECK(  boundFramebuffer->FillRectangle(  boundFramebuffer, x-half,     y-half,     lw + w, lw    ) ); // TOP
  DFB_CHECK(  boundFramebuffer->FillRectangle(  boundFramebuffer, x-half,     y-half + h, lw + w, lw    ) ); // BOTTOM
  DFB_CHECK(  boundFramebuffer->FillRectangle(  boundFramebuffer, x-half,     y-half,     lw,     lw + h) ); // LEFT
  DFB_CHECK(  boundFramebuffer->FillRectangle(  boundFramebuffer, x-half + w, y-half,     lw,     lw + h) ); // RIGHT

#ifndef DEBUG_SKIP_FLIPPING
  needsFlip = true;
#endif

}

static void drawImageTexture(float x, float y, float w, float h, pxTextureRef texture,
                             pxTextureRef mask,  bool useTextureDimsAlways, float* color,
                             pxConstantsStretch::constants xStretch,
                             pxConstantsStretch::constants yStretch)
{
  float iw = texture->width();
  float ih = texture->height();

#if 0
  // NEWER CODE
  //
  // THIS CAUSES SPURIOUS TEXT BELOW *PLACEHOLDER* TEXT...
  if( useTextureDimsAlways)
  {
      w = iw;
      h = ih;
  }
  else
  {
    if (w == -1)
      w = iw;
    if (h == -1)
      h = ih;
  }
#else
   // OLDER CODE
   if (iw <= 0 || ih <= 0)
   {
     rtLogError("ERROR: drawImageTexture() >>>  WxH: 0x0   BAD !");
     return;
   }
#endif

  if (xStretch == pxConstantsStretch::NONE)  w = iw;
  if (yStretch == pxConstantsStretch::NONE)  h = ih;

#if 0 // redundent for DFB
  const float verts[4][2] =
  {
    { x,     y },
    { x+w,   y },
    { x,   y+h },
    { x+w, y+h }
  };

  float tw;
  switch(xStretch) {
  case pxConstantsStretch::NONE:
  case pxConstantsStretch::STRETCH:
    tw = 1.0;
    break;
  case pxConstantsStretch::REPEAT:
    tw = w/iw;
    break;
  }

  float tb = 0;
  float th;
  switch(yStretch) {
  case pxConstantsStretch::NONE:
  case pxConstantsStretch::STRETCH:
    th = 1.0;
    break;
  case pxConstantsStretch::REPEAT:
#if 0 // PX_TEXTURE_ANCHOR_BOTTOM
    th = h/ih;
#else

    float temp = h/ih;
    th = ceil(temp);
    tb = 1.0f-(temp-floor(temp));
#endif
    break;
  }
#endif // 00

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if (!currentFramebuffer)
  {
    rtLogError("ERROR:  the currentFramebuffer surface is NULL !");
    return;
  }

  DFBRectangle src;

  src.x = 0;
  src.y = 0;
  src.w = iw;
  src.h = ih;

  DFBRectangle dst;

  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if 0
  // TEXTURED TRIANGLES...

  if(boundTexture != NULL)
  {
    pxFBOTexture *tex = (pxFBOTexture *) texture.getPtr();

    rtLogDebug("\nINFO: ###################  TextureTriangles()  !!!!!!!!!!!!!!!!");
    dfbSurface->TextureTriangles(dfbSurface, tex->getSurface(), tex->getVetricies(), NULL, 4, DTTF_STRIP);
  }
  else
  {
    rtLogDebug("\nINFO: ###################  TextureTriangles() fails");
  }
#endif

  if (mask.getPtr() == NULL)
  {
    if (texture->getType() != PX_TEXTURE_ALPHA)
    {
      draw_TEXTURE(gResW,gResH,gMatrix.data(),gAlpha,src,dst,texture,xStretch,yStretch);
    }
    else if (texture->getType() == PX_TEXTURE_ALPHA)
    {
      float colorPM[4];
      premultiply(colorPM,color);

      draw_SOLID(gResW,gResH,gMatrix.data(),gAlpha,src,dst,texture,colorPM);
    }
  }
  else
  {
      draw_MASK(gResW,gResH,gMatrix.data(),gAlpha,src,dst,texture,mask);
  }

#ifndef DEBUG_SKIP_FLIPPING
  needsFlip = true;
#endif

}

static void drawImage92(float x,  float y,  float w,  float h,
                        float x1, float y1, float x2, float y2, pxTextureRef texture)
{
  if (texture.getPtr() == NULL)
  {
    return;
  }

  DFBRectangle srcUL, dstUL; // Upper Left;
  DFBRectangle srcUM, dstUM; // Upper Middle;
  DFBRectangle srcUR, dstUR; // Upper Right

  DFBRectangle srcML, dstML; // Middle Left;
  DFBRectangle srcMM, dstMM; // Middle Middle;
  DFBRectangle srcMR, dstMR; // Middle Right

  DFBRectangle srcBL, dstBL; // Bottom Left;
  DFBRectangle srcBM, dstBM; // Bottom Middle;
  DFBRectangle srcBR, dstBR; // Bottom Right

  float iw = texture->width();
  float ih = texture->height();

  // HACK ! .. prevent 'zero' dimension rectangles getting to blitter.
  //
  if(x1 <=0) x1 = 1;
  if(y1 <=0) y1 = 1;
  if(x2 <=0) x2 = 1;
  if(y2 <=0) y2 = 1;

// printf("\n ###  drawImage92() >> WxH:  %f x %f     x1: %f  y1: %f    x2: %f y2: %f",w,h,  x1, y1,  x2, y2);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // TOP ROW
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  int k = 1; // HACK to prevent Zero dimensioned rectangles.

  srcUL.w = x1;                    srcUM.w = (x2 - x1) + k;         srcUR.w = iw - x2;
  srcUL.h = y1;                    srcUM.h = y1;                    srcUR.h = y1;
  srcUL.x = 0;                     srcUM.x = x1;                    srcUR.x = x2;
  srcUL.y = 0;                     srcUM.y = 0;                     srcUR.y = 0;

  dstUL.w = x1;                    dstUM.w = w - (iw - x2) - x1;    dstUR.w = (iw - x2);
  dstUL.h = y1;                    dstUM.h = y1;                    dstUR.h = y1;
  dstUL.x = x;                     dstUM.x = dstUL.x + dstUL.w;     dstUR.x = dstUM.x + dstUM.w;
  dstUL.y = y;                     dstUM.y = dstUL.y;               dstUR.y = dstUL.y;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // MIDDLE ROW
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  srcML.w = x1;                    srcMM.w = (x2 - x1) + k;        srcMR.w = (iw - x2);
  srcML.h = (y2 - y1) + k;         srcMM.h = (y2 - y1) + k;        srcMR.h = (y2 - y1) + k;
  srcML.x = 0;                     srcMM.x = x1;                   srcMR.x = x2;
  srcML.y = y1;                    srcMM.y = y1 ;                  srcMR.y = y1;

  dstML.w = dstUL.w;               dstMM.w = dstUM.w;              dstMR.w = dstUR.w;
  dstML.h = h - (ih - y2) - y1;    dstMM.h = dstML.h;              dstMR.h = dstML.h;
  dstML.x = dstUL.x;               dstMM.x = dstUM.x;              dstMR.x = dstUR.x;
  dstML.y = dstUL.y + dstUL.h;     dstMM.y = dstUM.y + dstUM.h;    dstMR.y = dstUR.y + dstUR.h;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // BOTTOM ROW
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  srcBL.w = x1;                    srcBM.w = (x2 - x1) + k;        srcBR.w = (iw - x2);
  srcBL.h = (ih - y2);             srcBM.h = (ih - y2);            srcBR.h = (ih - y2);
  srcBL.x = 0;                     srcBM.x = x1;                   srcBR.x = x2;
  srcBL.y = y2;                    srcBM.y = y2;                   srcBR.y = y2;

  dstBL.w = dstUL.w;               dstBM.w = dstMM.w;              dstBR.w = dstMR.w;
  dstBL.h = (ih - y2);             dstBM.h = dstBL.h;              dstBR.h = dstBL.h;
  dstBL.x = dstUL.x;               dstBM.x = dstMM.x;              dstBR.x = dstMR.x;
  dstBL.y = dstML.y + dstML.h;     dstBM.y = dstMM.y + dstMM.h;    dstBR.y = dstMR.y + dstMR.h;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if (boundFramebuffer == NULL)
  {
    rtLogError("ERROR: drawImage92() >>> boundFrame: NULL   BAD !");

    boundFramebuffer = dfbSurface;
  }

  texture->bindGLTexture(0);

  //                                   UPPER ROW              MIDDLE ROW            BOTTOM ROW
  const DFBRectangle src[9] = {srcUL, srcUM, srcUR,    srcML, srcMM, srcMR,    srcBL, srcBM, srcBR };
  const DFBRectangle dst[9] = {dstUL, dstUM, dstUR,    dstML, dstMM, dstMR,    dstBL, dstBM, dstBR };

#ifndef DEBUG_SKIP_BLIT
  DFB_CHECK(boundFramebuffer->BatchStretchBlit(boundFramebuffer, boundTexture, &src[0], &dst[0], 9));
#endif
}

bool gContextInit = false;

pxContext::~pxContext()
{
}

void pxContext::init()
{
#if 0
  if (gContextInit)
    return;
  else
    gContextInit = true;
#endif

#ifdef ENABLE_DFB_GENERIC
  dfb = outsideDfb;
  dfbSurface = outsideDfbSurface;
#endif //ENABLE_DFB_GENERIC

  DFB_CHECK( dfbSurface->Clear( dfbSurface, 0x00, 0x00, 0x00, 0x00 ) ); // TRANSPARENT

  boundFramebuffer = dfbSurface;  // needed here.
  boundTexture     = dfbSurface;  // needed here.

  DFB_CHECK(dfbSurface->SetRenderOptions(dfbSurface, DSRO_MATRIX));

  rtLogSetLevel(RT_LOG_INFO); // LOG LEVEL
}

void pxContext::setSize(int w, int h)
{
  gResW = w;
  gResH = h;

  if (currentFramebuffer == defaultFramebuffer)
  {
    defaultContextSurface.width  = w;
    defaultContextSurface.height = h;
  }
}

void pxContext::getSize(int& w, int& h)
{
   w = gResW;
   h = gResH;
}

void pxContext::clear(int /*w*/, int /*h*/)
{
  if(dfbSurface == 0)
  {
    rtLogError("cannot clear on context surface because surface is NULL");
    return;
  }

  DFB_CHECK( dfbSurface->Clear( dfbSurface, 0x00, 0x00, 0x00, 0x00 ) ); // TRANSPARENT
  //DFB_CHECK( dfbSurface->Clear( dfbSurface, 0x00, 0x00, 0x8F, 0xFF ) );  //  CLEAR_BLUE   << JUNK
}

void pxContext::clear(int /*w*/, int /*h*/, float* /*fillColor*/ )
{
  //TODO - support color
  if(dfbSurface == 0)
   {
      rtLogError("cannot clear on context surface because surface is NULL");
      return;
   }
  // TODO - apply color

  DFB_CHECK( dfbSurface->Clear( dfbSurface, 0x00, 0x00, 0x00, 0x00 ) ); // TRANSPARENT
}

void pxContext::clear(int left, int top, int right, int bottom)
{
  // TODO
}

void pxContext::setMatrix(pxMatrix4f& m)
{
 // printf("\n %s()" , __PRETTY_FUNCTION__);

  gMatrix.multiply(m);

  const float *mm = gMatrix.data();

  s32 matrix[9];

  // Convert to fixed point for DFB...
  matrix[0] = (s32)(mm[0]  * 0x10000);
  matrix[1] = (s32)(mm[4]  * 0x10000);
  matrix[2] = (s32)(mm[12] * 0x10000);
  matrix[3] = (s32)(mm[1]  * 0x10000);
  matrix[4] = (s32)(mm[5]  * 0x10000);
  matrix[5] = (s32)(mm[13] * 0x10000);

  matrix[6] = 0x00000;
  matrix[7] = 0x00000;
  matrix[8] = 0x10000;

//   DFB_CHECK(/*dfbSurface*/ boundTexture->SetRenderOptions(/*dfbSurface*/ boundTexture, DSRO_MATRIX)); // JUNK

  DFB_CHECK(dfbSurface->SetMatrix(dfbSurface, matrix));
}

pxMatrix4f pxContext::getMatrix()
{
  return gMatrix;
}

void pxContext::setAlpha(float a)
{
  gAlpha *= a;
}

float pxContext::getAlpha()
{
  return gAlpha;
}

pxContextFramebufferRef pxContext::createFramebuffer(int w, int h)
{
  pxContextFramebuffer *fbo     = new pxContextFramebuffer();
  pxFBOTexture         *texture = new pxFBOTexture();

  texture->createTexture(w, h);

  fbo->setTexture(texture);

  return fbo;
}

pxError pxContext::updateFramebuffer(pxContextFramebufferRef fbo, int w, int h)
{
  if (fbo.getPtr() == NULL || fbo->getTexture().getPtr() == NULL)
  {
    return PX_FAIL;
  }

  return fbo->getTexture()->resizeTexture(w, h);
}

pxContextFramebufferRef pxContext::getCurrentFramebuffer()
{
  return currentFramebuffer;
}

pxError pxContext::setFramebuffer(pxContextFramebufferRef fbo)
{
  if (fbo.getPtr() == NULL || fbo->getTexture().getPtr() == NULL)
  {
    //glViewport ( 0, 0, defaultContextSurface.width, defaultContextSurface.height);

    gResW = defaultContextSurface.width;
    gResH = defaultContextSurface.height;

    // TODO probably need to save off the original FBO handle rather than assuming zero
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);  TRACK_FBO_CALLS();

    boundFramebuffer   = dfbSurface; //default
    currentFramebuffer = defaultFramebuffer;

    pxContextState contextState;
    currentFramebuffer->currentState(contextState);

    gAlpha  = contextState.alpha;
    gMatrix = contextState.matrix;

    return PX_OK;
  }

  currentFramebuffer = fbo;

  pxContextState contextState;
  currentFramebuffer->currentState(contextState);

  gAlpha  = contextState.alpha;
  gMatrix = contextState.matrix;

  return fbo->getTexture()->prepareForRendering();
}

#if 0
pxError pxContext::deleteContextSurface(pxTextureRef texture)
{
  if (texture.getPtr() == NULL)
  {
    return PX_FAIL;
  }
  return texture->deleteTexture();
}
#endif

void pxContext::drawRect(float w, float h, float lineWidth, float* fillColor, float* lineColor)
{
#ifdef DEBUG_SKIP_RECT
#warning "DEBUG_SKIP_RECT enabled ... Skipping "
  return;
#endif

  if(gAlpha == 0.0 || fillColor == NULL || fillColor[3] == 0.0 || lineColor == NULL || lineColor[3] == 0.0 || lineWidth == 0.0)
  {
   // printf("\n drawRect() - INVISIBLE");
    return;
  }

  //printf("\n %s() - ENTER", __PRETTY_FUNCTION__); // JUNK

  if(fillColor == NULL && lineColor == NULL)
  {
    rtLogError("cannot drawRect() on context surface because colors are NULL");
    return;
  }

  if(boundTexture == NULL)
  {
    rtLogError("cannot drawRect() on context surface because boundTexture is NULL");
    return;
  }

  if(fillColor != NULL && fillColor[3] > 0.0) // with non-transparent color
  {
    // Draw FILL rectangle for smaller FILL areas
    float half = lineWidth/2;
    drawRect2(half, half, w-lineWidth, h-lineWidth, fillColor);
  }

  if(lineWidth > 0 && lineColor != NULL && lineColor[3] > 0.0) // with non-transparent color and non-zero stroke
  {
    drawRectOutline(0, 0, w, h, lineWidth, lineColor);
  }
}

void pxContext::drawImage9(float w, float h, float x1, float y1,
                           float x2, float y2, pxTextureRef texture)
{
#ifdef DEBUG_SKIP_IMAGE9
#warning "DEBUG_SKIP_IMAGE9 enabled ... Skipping "
  return;
#endif

  if(gAlpha == 0.0)
  {
    printf("\n drawImage9() - INVISIBLE");
    return;
  }

  if (texture.getPtr() != NULL)
  {
    drawImage92(0, 0, w, h, x1, y1, x2, y2, texture);
  }
}

void pxContext::drawImage(float x, float y, float w, float h,
                        pxTextureRef t, pxTextureRef mask,
                        bool useTextureDimsAlways, float* color,
                        pxConstantsStretch::constants stretchX,
                        pxConstantsStretch::constants stretchY)
{
#ifdef DEBUG_SKIP_IMAGE
#warning "DEBUG_SKIP_IMAGE enabled ... Skipping "
  return;
#endif

  if(gAlpha == 0.0)
  {
    //printf("\n drawImage() - INVISIBLE");
    return;
  }

  if (t.getPtr() == NULL)
  {
    rtLogError("ERROR: pxContext::drawImage() >>> texture: NULL   BAD !");
    return;
  }

  float black[4] = {0,0,0,1};
  drawImageTexture(x, y, w, h, t, mask, useTextureDimsAlways,
                  color?color:black, stretchX, stretchY);
}

void pxContext::drawDiagRect(float x, float y, float w, float h, float* color)
{
#ifdef DEBUG_SKIP_DIAG_RECT
#warning "DEBUG_SKIP_DIAG_RECT enabled ... Skipping "
   return;
#endif

  if(gAlpha == 0.0)
  {
    //printf("\n drawImage() - INVISIBLE");
    return;
  }

  if (!mShowOutlines) return;

  if(boundFramebuffer == NULL)
  {
    rtLogError("cannot drawDiagRect() on context surface because boundTexture is NULL");
    return;
  }

  if(w == 0.0 || h == 0.0)
  {
    rtLogError("cannot drawDiagRect() - width/height cannot be Zero.");
    return;
  }

  if (!boundFramebuffer)
  {
    rtLogError("ERROR:  the boundFramebuffer surface is NULL !");

    boundFramebuffer = dfbSurface; // default
  }

  /*
  const float verts[4][2] =
  {
    { x,   y   },
    { x+w, y   },
    { x+w, y+h },
    { x,   y+h },
  };
  */

  float colorPM[4];
  premultiply(colorPM, color);

  DFB_CHECK (boundFramebuffer->SetColor(boundFramebuffer, colorPM[0] * 255.0, // RGBA
                                                          colorPM[1] * 255.0,
                                                          colorPM[2] * 255.0,
                                                          colorPM[3] * 255.0));

  DFB_CHECK (boundFramebuffer->DrawRectangle(boundFramebuffer, x, y, w, h));
}

void pxContext::drawDiagLine(float x1, float y1, float x2, float y2, float* color)
{
#ifdef DEBUG_SKIP_DIAG_LINE
#warning "DEBUG_SKIP_DIAG_LINE enabled ... Skipping "
   return;
#endif

  if (!mShowOutlines) return;

  if(gAlpha == 0.0)
  {
    printf("\n drawImage() - INVISIBLE");
    return;
  }

  if(boundTexture == NULL)
  {
    rtLogError("cannot drawDiagLine() on context surface because boundTexture is NULL");
    return;
  }

  if (!boundFramebuffer)
  {
    rtLogError("ERROR:  the boundFramebuffer surface is NULL !");

    boundFramebuffer = dfbSurface; // default
  }

  /*
  const float verts[4][2] =
  {
    { x1, y1 },
    { x2, y2 },
  };
  */

  float colorPM[4];
  premultiply(colorPM,color);

  DFB_CHECK (boundFramebuffer->SetColor(boundTexture, colorPM[0] * 255.0, // RGBA
                                                      colorPM[1] * 255.0,
                                                      colorPM[2] * 255.0,
                                                      colorPM[3] * 255.0));

  DFB_CHECK (boundFramebuffer->DrawLine(boundFramebuffer, x1,y1, x2,y2));
}

pxTextureRef pxContext::createTexture(pxOffscreen& o)
{
  pxTextureOffscreen* offscreenTexture = new pxTextureOffscreen(o);

  return offscreenTexture;
}

pxTextureRef pxContext::createTexture(float w, float h, float iw, float ih, void* buffer)
{
  pxTextureAlpha* alphaTexture = new pxTextureAlpha(w,h,iw,ih,buffer);

  return alphaTexture;
}

void pxContext::pushState()
{
  pxContextState contextState;

  contextState.matrix = gMatrix;
  contextState.alpha  = gAlpha;

  currentFramebuffer->pushState(contextState);
}

void pxContext::popState()
{
  pxContextState contextState;

  if (currentFramebuffer->popState(contextState) == PX_OK)
  {
    gAlpha  = contextState.alpha;
    gMatrix = contextState.matrix;
  }
}

void pxContext::snapshot(pxOffscreen& o)
{
  o.init(gResW,gResH);
  // TODO
}

void pxContext::mapToScreenCoordinates(float inX, float inY, int &outX, int &outY)
{
  pxVector4f positionVector(inX, inY, 0, 1);
  pxVector4f positionCoords = gMatrix.multiply(positionVector);

  if (positionCoords.w() == 0)
  {
    outX = positionCoords.x();
    outY = positionCoords.y();
  }
  else
  {
    outX = positionCoords.x() / positionCoords.w();
    outY = positionCoords.y() / positionCoords.w();
  }
}

void pxContext::mapToScreenCoordinates(pxMatrix4f& m, float inX, float inY, int &outX, int &outY)
{
  pxVector4f positionVector(inX, inY, 0, 1);
  pxVector4f positionCoords = m.multiply(positionVector);

  if (positionCoords.w() == 0)
  {
    outX = positionCoords.x();
    outY = positionCoords.y();
  }
  else
  {
    outX = positionCoords.x() / positionCoords.w();
    outY = positionCoords.y() / positionCoords.w();
  }
}

void pxContext::enableDirtyRectangles(bool enable)
{
  currentFramebuffer->enableDirtyRectangle(enable);
}

bool pxContext::isObjectOnScreen(float /*x*/, float /*y*/, float /*width*/, float /*height*/)
{
#if 1
  return true;
#else
  int outX1, outX2, outY1, outY2;
  mapToScreenCoordinates(width,height,outX2, outY2);
  mapToScreenCoordinates(x,y,outX1, outY1);
  int fboWidth = currentFramebuffer->width();
  int fboHeight = currentFramebuffer->height();
  if (currentFramebuffer == defaultFramebuffer)
  {
    fboWidth = gResW;
    fboHeight = gResH;
  }
  if ((outX1 < 0 && outX2 < 0) ||
      (outX1 > fboWidth && outX2 > fboWidth) ||
      (outY1 < 0 && outY2 < 0) ||
      (outY1 > fboHeight && outY2 > fboHeight))
  {
    return false;
  }
  return true;
#endif
}

#if 0

// JUNK
// JUNK
// JUNK
// JUNK

class Timer
{
   timeval timer[2];

   struct timespec start_tm, end_tm;

public:

   void start()
   {
      gettimeofday(&this->timer[0], NULL);
   }

   void stop()
   {
      gettimeofday(&this->timer[1], NULL);
   }

   long duration_ms() const // milliseconds
   {
      long secs( this->timer[1].tv_sec  - this->timer[0].tv_sec);
      long usecs(this->timer[1].tv_usec - this->timer[0].tv_usec);

      if(usecs < 0)
      {
         --secs;
         usecs += 1000000;
      }

      return static_cast<long>( (secs * 1000) + ((double) usecs / 1000.0 + 0.5));
   }
};

// JUNK
// JUNK
// JUNK
// JUNK

//static uint64_t sum = 0;
//static unsigned int blits = 0, fps = 0;
//static Timer timer;
#endif //0
