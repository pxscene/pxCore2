#include "rtDefs.h"
#include "rtLog.h"
#include "pxContext.h"

#include <directfb.h>

/* Place holder for a dfb implementation of pxContext */


extern IDirectFB              *dfb;
extern IDirectFBDisplayLayer  *layer;

extern IDirectFBSurface       *wsurface;
extern IDirectFBEventBuffer   *wbuffer;


#define DFB_CHECK(x...)             \
{                                   \
  DFBResult err = x;                \
                                    \
  if (err != DFB_OK)                \
{                                   \
  rtLogError( " DFB died... " );    \
  DirectFBErrorFatal( #x, err );    \
  }                                 \
}

pxContextSurfaceNativeDesc  defaultContextSurface;
pxContextSurfaceNativeDesc* currentContextSurface = &defaultContextSurface;

static IDirectFBSurface       *boundSurface;
static DFBSurfaceDescription   boundDesc;

pxTextureRef defaultRenderSurface;
pxTextureRef currentRenderSurface = defaultRenderSurface;

struct point
{
  float x;
  float y;
  float s;
  float t;
};



//====================================================================================================================================================================================

/*
class pxSurfaceTexture : public pxTexture
{
public:
  pxSurfaceTexture() : mWidth(0), mHeight(0), mFramebufferId(0), mTextureId(0)
  {
    mTextureType = PX_TEXTURE_FRAME_BUFFER;
  }

  ~pxSurfaceTexture() { deleteTexture(); }

  void createTexture(int width, int height)
  {
    if (mFramebufferId != 0 && mTextureId != 0)
    {
      deleteTexture();
    }

    mWidth = width;
    mHeight = height;


//    glGenFramebuffers(1, &mFramebufferId);
//    glGenTextures(1, &mTextureId);
//    glActiveTexture(GL_TEXTURE3);
//    glBindTexture(GL_TEXTURE_2D, mTextureId);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
//           width, height, 0, GL_BGRA_EXT,
//           GL_UNSIGNED_BYTE, NULL);

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  }

  virtual pxError deleteTexture()
  {

//    if (mFramebufferId!= 0)
//    {
//      glDeleteFramebuffers(1, &mFramebufferId);
//      mFramebufferId = 0;
//    }

//    if (mTextureId != 0)
//    {
//      glDeleteTextures(1, &mTextureId);
//      mTextureId = 0;
//    }

    return PX_OK;
  }

  virtual pxError prepareForRendering()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, mTextureId, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      if ((mWidth != 0) && (mHeight != 0))
      {
        rtLogWarn("error setting the render surface");
      }
      return PX_FAIL;
    }

//    glActiveTexture(GL_TEXTURE3);
//    glBindTexture(GL_TEXTURE_2D, mTextureId);
//    glViewport ( 0, 0, mWidth, mHeight);
//    glUniform2f(u_resolution, mWidth, mHeight);

    return PX_OK;
  }

  virtual pxError bindTexture()
  {

//    if (mFramebufferId == 0 || mTextureId == 0)
//    {
//      return PX_NOTINITIALIZED;
//    }

//    glActiveTexture(GL_TEXTURE3);
//    glBindTexture(GL_TEXTURE_2D, mTextureId);

//    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
//    glUniform1i(u_texture, 3);

    return PX_OK;
  }
  
  virtual pxError bindTextureAsMask()
  {
    return PX_FAIL;
  }
  
  virtual pxError getOffscreen(pxOffscreen& o)
  {
    (void)o;
    // TODO
    return PX_FAIL;
  }

  virtual float width()  { return mWidth; }
  virtual float height() { return mHeight; }

private:
  float mWidth;
  float mHeight;
//  GLuint mFramebufferId;
//  GLuint mTextureId;
  IDirectFBSurface       *surface;
  DFBSurfaceDescription   dsc;
};

*/

//====================================================================================================================================================================================

class pxTextureOffscreen : public pxTexture
{
public:
  pxTextureOffscreen() : mOffscreen(), mInitialized(false)
  {
    mTextureType = PX_TEXTURE_OFFSCREEN;
  }

  pxTextureOffscreen(pxOffscreen& o) : mOffscreen(), mInitialized(false)
  {
    mTextureType = PX_TEXTURE_OFFSCREEN;

    printf("\n############# this: %p >>  pxTextureOffscreen()",this);

    createTexture(o);
  }

  ~pxTextureOffscreen()  {  deleteTexture(); }

  void createTexture(pxOffscreen& o)
  {

    o.swizzleTo(PX_PIXEL_FMT_ABGR);  // <<< DSPF_ABGR

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Load DFB surface with TEXTURE

    dsc.width                 = o.width();
    dsc.height                = o.height();
    dsc.flags                 = (DFBSurfaceDescriptionFlags)(DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT);
    dsc.caps                  = DSCAPS_NONE;
    dsc.pixelformat           = DSPF_ABGR; //DSPF_ARGB;
    dsc.preallocated[0].data  = o.base();      // Buffer is your data
    dsc.preallocated[0].pitch = o.width()*4;
    dsc.preallocated[1].data  = NULL;          // Not using a back buffer
    dsc.preallocated[1].pitch = 0;

    DFB_CHECK (dfb->CreateSurface( dfb, &dsc, &surface));

    printf("\n############# this: %p >>  createTexture()  wsurface: %p  surface: %p    WxH: %d x %d - DONE",
           this, wsurface, surface, dsc.width, dsc.height);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    mInitialized = true;
  }

  virtual pxError deleteTexture()
  {
    printf("\n############# this: %p >>  deleteTexture() ", this);

    if(surface)
    {
      if(surface == boundSurface)
      {
        // Is this sufficient ? or Dangerous ?
        boundSurface = NULL;
      }
      surface->Release(surface);

      surface = NULL;
    }

    mInitialized = false;
    return PX_OK;
  }

  virtual pxError bindTexture()
  {
    if (!mInitialized)
    {
      printf("\n############# this: %p >>  bindTexture()  <<< PX_NOTINITIALIZED", this);

      return PX_NOTINITIALIZED;
    }

    boundDesc    = dsc;
    boundSurface = surface;

//    printf("\n#############  this: %p >>  bindTexture() >> boundSurface: %p  xy: (%d,%d)   WxH: %d x %d ",
//           this ,boundSurface, 0,0, boundDesc.width, boundDesc.height);

    return PX_OK;
  }

  virtual pxError bindTextureAsMask()
  {
    return PX_FAIL;
  }

  virtual pxError getOffscreen(pxOffscreen& o)
  {
    printf("\n############# this: %p >>  getOffscreen()  ENTER", this);

//    if (!mInitialized)
//    {
//      return PX_NOTINITIALIZED;
//    }
//    o.init(mOffscreen.width(), mOffscreen.height());
//    mOffscreen.blit(o);
    return PX_OK;
  }

  virtual float width()  { return dsc.width;  }
  virtual float height() { return dsc.height; }

private:
  pxOffscreen mOffscreen;
  bool        mInitialized;

  IDirectFBSurface       *surface;
  DFBSurfaceDescription   dsc;

};// CLASS - pxTextureOffscreen

//====================================================================================================================================================================================

class pxTextureAlpha : public pxTexture
{
public:
  pxTextureAlpha() : mDrawWidth(0.0), mDrawHeight (0.0), mImageWidth(0.0),
                     mImageHeight(0.0), /*mTextureId(0),*/ mInitialized(false)
  {
    mTextureType = PX_TEXTURE_ALPHA;
  }

  pxTextureAlpha(float w, float h, float iw, float ih, void* buffer) 
    : mDrawWidth(w), mDrawHeight (h), mImageWidth(iw),
      mImageHeight(ih), /*mTextureId(0),*/ mInitialized(false), mBuffer(NULL)
  {
    mTextureType = PX_TEXTURE_ALPHA;
    // copy the pixels
    int bitmapSize = ih*iw;
    mBuffer = malloc(bitmapSize);
    memcpy(mBuffer, buffer, bitmapSize);
// TODO Moved this to bindTexture because of more pain from JS thread calls
//    createTexture(w, h, iw, ih);
  }

  ~pxTextureAlpha() 
  { 
    if(mBuffer) 
      free(mBuffer);
    deleteTexture(); 
  }

  void createTexture(float w, float h, float iw, float ih)
  {
//    if (mTextureId != 0)
//    {
//      deleteTexture();
//    }
//    glGenTextures(1, &mTextureId);

//    mDrawWidth = w;
//    mDrawHeight = h;
//    mImageWidth = iw;
//    mImageHeight = ih;

//    glActiveTexture(GL_TEXTURE1);
//    glBindTexture(GL_TEXTURE_2D, mTextureId);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//    glTexImage2D(
//      GL_TEXTURE_2D,
//      0,
//      GL_ALPHA,
//      iw,
//      ih,
//      0,
//      GL_ALPHA,
//      GL_UNSIGNED_BYTE,
//      mBuffer
//    );
    mInitialized = true;
  }

  virtual pxError deleteTexture()
  {
//    if (mTextureId != 0)
//    {
//      glDeleteTextures(1, &mTextureId);
//      mTextureId = 0;
//    }
    mInitialized = false;
    return PX_OK;
  }

  virtual pxError bindTexture()
  {
    // TODO Moved to here because of js threading issues
    if (!mInitialized) createTexture(mDrawWidth,mDrawHeight,mImageWidth,mImageHeight);
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }

//    glActiveTexture(GL_TEXTURE1);
//    glBindTexture(GL_TEXTURE_2D, mTextureId);
//    glUniform1i(u_texture, 1);

//    glUniform1f(u_alphatexture, 2.0);

    return PX_OK;
  }

  virtual pxError bindTextureAsMask()
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }

//    glActiveTexture(GL_TEXTURE2);
//    glBindTexture(GL_TEXTURE_2D, mTextureId);
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

//    glUniform1i(u_texture, 1);
//    glUniform1f(u_alphatexture, 2.0);
////    glUniform4fv(u_color, 1, mColor);

//    glUniform1i(u_mask, 2);
//    glUniform1i(u_enablemask, 1);

    return PX_OK;
  }

  virtual pxError getOffscreen(pxOffscreen& o)
  {
    (void)o;
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }
    return PX_FAIL;
  }

  virtual float width() { return mDrawWidth; }
  virtual float height() { return mDrawHeight; }

private:
  float mDrawWidth;
  float mDrawHeight;
  float mImageWidth;
  float mImageHeight;
  //GLuint mTextureId;
  bool mInitialized;
  void* mBuffer;
  bool mUploaded;
}; // CLASS - pxTextureAlpha


//====================================================================================================================================================================================

static void draw9SliceRect(float x, float y, float w, float h, float x1, float y1, float x2, float y2)
{
  float ox1 = x;
  float ix1 = x+x1;
  float ox2 = x+w;
  float ix2 = x+w-x2;
  float oy1 = y;
  float iy1 = y+y1;
  float oy2 = y+h;
  float iy2 = y+h-y2;

  const float verts[22][2] =
  {
    { ox1,oy1 },
    { ix1,oy1 },
    { ox1,iy1 },
    { ix1,iy1 },
    { ox1,iy2 },
    { ix1,iy2 },
    { ox1,oy2 },
    { ix1,oy2 },
    { ix2,oy2 },
    { ix1,iy2 },
    { ix2,iy2 },
    { ix1,iy1 },
    { ix2,iy1 },
    { ix1,oy1 },
    { ix2,oy1 },
    { ox2,oy1 },
    { ix2,iy1 },
    { ox2,iy1 },
    { ix2,iy2 },
    { ox2,iy2 },
    { ix2,oy2 },
    { ox2,oy2 }
  };
#if 0
  const float colors[4][3] =
  {
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 0, 1, 0 },
    { 0, 0, 1 }
  };
  const float uv[22][2] =
  {
    { 0, 0 },
    { 1, 0 },
    { 0, 1 },
    { 1, 1 }
  };
#endif


  //  {
  //    glUniform1f(u_alphatexture, 0.0);
  //    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
  //    glEnableVertexAttribArray(attr_pos);
  //    glDrawArrays(GL_TRIANGLE_STRIP, 0, 22);
  //    glDisableVertexAttribArray(attr_pos);
  //  }
}


static void drawRect2(float x, float y, float w, float h)
{
  if(wsurface == 0)
  {
    rtLogError("cannot drawRect2 on context surface because surface is NULL");
    return;
  }

  DFB_CHECK (wsurface->FillRectangle (wsurface, x, y, w, h));
}


static void drawRectOutline(float x, float y, float w, float h, float lw)
{
  if(wsurface == 0)
  {
    rtLogError("cannot drawRect2 on context surface because surface is NULL");
    return;
  }

  //DFB_CHECK (wsurface->DrawRectangle (wsurface, x, y, w, h));

  //  float half = lw/2;
  //  float ox1  = x-half;
  //  float ix1  = x+half;
  //  float ox2  = x+w+half;
  //  float ix2  = x+w-half;
  //  float oy1  = y-half;
  //  float iy1  = y+half;
  //  float oy2  = y+h+half;
  //  float iy2  = y+h-half;

  //  const float verts[10][2] =
  //  {
  //    { ox1,oy1 },
  //    { ix1,iy1 },
  //    { ox2,oy1 },
  //    { ix2,iy1 },
  //    { ox2,oy2 },
  //    { ix2,iy2 },
  //    { ox1,oy2 },
  //    { ix1,iy2 },
  //    { ox1,oy1 },
  //    { ix1,iy1 }
  //  };

  //  {
  //    glUniform1f(u_alphatexture, 0.0);

  //    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
  //    glEnableVertexAttribArray(attr_pos);
  //    glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
  //    glDisableVertexAttribArray(attr_pos);
  //  }
}


static void drawSurface2(float x, float y, float w, float h, pxContextSurfaceNativeDesc* contextSurface)
{
  printf("\nINFO: ###################  drawSurface2()  ");

  if ((contextSurface == NULL) || (contextSurface->surface == NULL))
  {
    return;
  }

  //  glActiveTexture(GL_TEXTURE0);
  //  glBindTexture(GL_TEXTURE_2D, contextSurface->texture);
  //  glUniform1i(u_texture, 0);

  //  const float verts[4][2] =
  //  {
  //    { x,y },
  //    {  x+w, y },
  //    {  x,  y+h },
  //    {  x+w, y+h }
  //  };

  //  const float uv[4][2] =
  //  {
  //    { 0, 0 },
  //    { 1, 0 },
  //    { 0, 1 },
  //    { 1, 1 }
  //  };

  //  {
  //    glUniform1f(u_alphatexture, 1.0);
  //    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
  //    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
  //    glEnableVertexAttribArray(attr_pos);
  //    glEnableVertexAttribArray(attr_uv);
  //    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  //    glDisableVertexAttribArray(attr_pos);
  //    glDisableVertexAttribArray(attr_uv);
  //  }

  //  glBindTexture(GL_TEXTURE_2D, textureId1); //bind back to original texture
}


static void drawImage2(float x, float y, float w, float h, pxOffscreen& offscreen,
                       pxStretch xStretch, pxStretch yStretch)
{
  printf("\nINFO: ###################  drawImage2()  ");

  //  glActiveTexture(GL_TEXTURE0);
  //  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
  //	       offscreen.width(), offscreen.height(), 0, GL_BGRA_EXT,
  //	       GL_UNSIGNED_BYTE, offscreen.base());

  //  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  //  glActiveTexture(GL_TEXTURE0);
  //  glUniform1i(u_texture, 0);

  //  const float verts[4][2] =
  //  {
  //    { x,y },
  //    {  x+w, y },
  //    {  x,  y+h },
  //    {  x+w, y+h }
  //  };

  //  const float uv[4][2] =
  //  {
  //    { 0, 0 },
  //    { 1, 0 },
  //    { 0, 1 },
  //    { 1, 1 }
  //  };

  //  {
  //    glUniform1f(u_alphatexture, 1.0);
  //    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
  //    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
  //    glEnableVertexAttribArray(attr_pos);
  //    glEnableVertexAttribArray(attr_uv);
  //    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  //    glDisableVertexAttribArray(attr_pos);
  //    glDisableVertexAttribArray(attr_uv);
  //  }
}


static void drawImageTexture(float x, float y, float w, float h, pxTextureRef texture,
                             pxTextureRef mask, pxStretch xStretch, pxStretch yStretch)
{
  if (texture.getPtr() == NULL)
  {
    //printf("\nINFO: ###################  nada");
    return;
  }

//  printf("\nINFO: ###################  drawImageTexture() texture: %p WxH: %.0f x %.0f", texture.getPtr(), texture->width(), texture->height());

  texture->bindTexture();

//  printf("\nINFO: ###################  drawImageTexture() texture: %p WxH: %.0f x %.0f", texture.getPtr(), texture->width(), texture->height());

  if (mask.getPtr() != NULL && mask->getType() == PX_TEXTURE_ALPHA)
  {
    printf("\nINFO: ###################  drawImageTexture() texture: %p WxH: %.0f x %.0f MASK MASK MASK", texture.getPtr(), texture->width(), texture->height());
    mask->bindTexture();
  }

  float iw = texture->width();
  float ih = texture->height();

  if (xStretch == PX_NONE)
  {
    w = iw;
  }

  if (yStretch == PX_NONE)
  {
    h = ih;
  }

  const float verts[4][2] =
  {
    { x,   y   },
    { x+w, y   },
    { x,   y+h },
    { x+w, y+h }
  };

  float tw = 1.0;
  switch(xStretch)
  {
    case PX_NONE:
    case PX_STRETCH:
      tw = 1.0;
      break;
    case PX_REPEAT:
      tw = w/iw;
      break;
  }

  float th = 1.0;
  switch(yStretch)
  {
    case PX_NONE:
    case PX_STRETCH:
      th = 1.0;
      break;
    case PX_REPEAT:
      th = h/ih;
      break;
  }

  float firstTextureY  = 0;
  float secondTextureY = th;

  if (texture->getType() == PX_TEXTURE_FRAME_BUFFER)
  {
    //opengl renders to a framebuffer in reverse y coordinates than pxContext renders.
    //the texture y values need to be flipped
    firstTextureY  = th;
    secondTextureY = 0;
  }

  const float uv[4][2] =
  {
    { 0,  firstTextureY  },
    { tw, firstTextureY  },
    { 0,  secondTextureY },
    { tw, secondTextureY }
  };

  //  {
  //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //    glUniform1f(u_alphatexture, 1.0);
  //    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
  //    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
  //    glEnableVertexAttribArray(attr_pos);
  //    glEnableVertexAttribArray(attr_uv);
  //    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  //    glDisableVertexAttribArray(attr_pos);
  //    glDisableVertexAttribArray(attr_uv);
  //  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  DFBRectangle rcSrc;

  rcSrc.x = 0;
  rcSrc.y = 0;
  rcSrc.w = boundDesc.width;
  rcSrc.h = boundDesc.height;

  DFBRectangle rcDst;

  rcDst.x = x;
  rcDst.y = y;
  rcDst.w = w;
  rcDst.h = h;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if 0
  printf("\n#############  wsurface: %p    boundSurface: %p", wsurface, boundSurface);
  printf("\n#############  rcSrc: (%d,%d)   WxH: %d x %d ",rcSrc.x, rcSrc.y, rcSrc.w, rcSrc.h);
  printf("\n#############  rcDst: (%d,%d)   WxH: %d x %d ",rcDst.x, rcDst.y, rcDst.w, rcDst.h);
#endif


//  DFB_CHECK(wsurface->Clear( wsurface, 0xFF, 0x00, 0x00, 0xFF ) );
//  DFB_CHECK(wsurface->Flip(wsurface, NULL, DSFLIP_WAITFORSYNC));

//    wsurface->Clear( wsurface, 0x00, 0x80, 0x00,  0xff );
//    wsurface->Flip( wsurface, NULL, (DFBSurfaceFlipFlags) 0 );

//  DFB_CHECK( wsurface->Clear(         wsurface, 0x00, 0x00, 0x00, 0xFF ) );   // Clear to BLACK
//  DFB_CHECK( wsurface->SetColor(      wsurface, 0x00, 0x00, 0xff, 0xff) );  // Rect in BLUE
//  DFB_CHECK( wsurface->FillRectangle (wsurface, 0, 0, 55, 55)     );

 // DFB_CHECK( wsurface->Blit(wsurface, boundSurface, NULL, x, y));

  DFB_CHECK(wsurface->StretchBlit(wsurface, boundSurface, &rcSrc, &rcDst));

 // DFB_CHECK( wsurface->Flip(wsurface, NULL, (DFBSurfaceFlipFlags) DSFLIP_WAITFORSYNC) ); //DSFLIP_WAITFORSYNC));
}


static void drawImage92(float x, float y, float w, float h, float x1, float y1, float x2, float y2,
                        pxOffscreen& offscreen)
{
  //  glActiveTexture(GL_TEXTURE0);
  //  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
  //	       offscreen.width(), offscreen.height(), 0, GL_BGRA_EXT,
  //	       GL_UNSIGNED_BYTE, offscreen.base());

  //  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  //  glActiveTexture(GL_TEXTURE0);
  //  glUniform1i(u_texture, 0);

  //  float ox1 = x;
  //  float ix1 = x+x1;
  //  float ix2 = x+w-x2;
  //  float ox2 = x+w;

  //  float oy1 = y;
  //  float iy1 = y+y1;
  //  float iy2 = y+h-y2;
  //  float oy2 = y+h;



  //  float w2 = offscreen.width();
  //  float h2 = offscreen.height();

  //  float ou1 = 0;
  //  float iu1 = x1/w2;
  //  float iu2 = (w2-x2)/w2;
  //  float ou2 = 1;

  //  float ov1 = 0;
  //  float iv1 = y1/h2;
  //  float iv2 = (h2-y2)/h2;
  //  float ov2 = 1;

  //#if 1 // sanitize values
  //  iu1 = pxClamp<float>(iu1, 0, 1);
  //  iu2 = pxClamp<float>(iu2, 0, 1);
  //  iv1 = pxClamp<float>(iv1, 0, 1);
  //  iv2 = pxClamp<float>(iv2, 0, 1);

  //  float tmin, tmax;

  //  tmin = pxMin<float>(iu1, iu2);
  //  tmax = pxMax<float>(iu1, iu2);
  //  iu1 = tmin;
  //  iu2 = tmax;

  //  tmin = pxMin<float>(iv1, iv2);
  //  tmax = pxMax<float>(iv1, iv2);
  //  iv1 = tmin;
  //  iv2 = tmax;

  //#endif

  //  const float verts[22][2] =
  //  {
  //    { ox1,oy1 },
  //    { ix1,oy1 },
  //    { ox1,iy1 },
  //    { ix1,iy1 },
  //    { ox1,iy2 },
  //    { ix1,iy2 },
  //    { ox1,oy2 },
  //    { ix1,oy2 },
  //    { ix2,oy2 },
  //    { ix1,iy2 },
  //    { ix2,iy2 },
  //    { ix1,iy1 },
  //    { ix2,iy1 },
  //    { ix1,oy1 },
  //    { ix2,oy1 },
  //    { ox2,oy1 },
  //    { ix2,iy1 },
  //    { ox2,iy1 },
  //    { ix2,iy2 },
  //    { ox2,iy2 },
  //    { ix2,oy2 },
  //    { ox2,oy2 }
  //  };

  //  const float uv[22][2] =
  //  {
  //    { ou1,ov1 },
  //    { iu1,ov1 },
  //    { ou1,iv1 },
  //    { iu1,iv1 },
  //    { ou1,iv2 },
  //    { iu1,iv2 },
  //    { ou1,ov2 },
  //    { iu1,ov2 },
  //    { iu2,ov2 },
  //    { iu1,iv2 },
  //    { iu2,iv2 },
  //    { iu1,iv1 },
  //    { iu2,iv1 },
  //    { iu1,ov1 },
  //    { iu2,ov1 },
  //    { ou2,ov1 },
  //    { iu2,iv1 },
  //    { ou2,iv1 },
  //    { iu2,iv2 },
  //    { ou2,iv2 },
  //    { iu2,ov2 },
  //    { ou2,ov2 }
  //  };

  //  {
  //    glUniform1f(u_alphatexture, 1.0);
  //    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
  //    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
  //    glEnableVertexAttribArray(attr_pos);
  //    glEnableVertexAttribArray(attr_uv);

  //    glDrawArrays(GL_TRIANGLE_STRIP, 0, 22);

  //    glDisableVertexAttribArray(attr_pos);
  //    glDisableVertexAttribArray(attr_uv);
  //  }
}


void pxContext::init()
{
  // TODO
}

void pxContext::setSize(int w, int h)
{
  //  glViewport(0, 0, (GLint)w, (GLint)h);
}


void pxContext::clear(int w, int h)
{
  if(wsurface == 0)
  {
    rtLogError("cannot clear on context surface because surface is NULL");
    return;
  }

  DFB_CHECK( wsurface->Clear( wsurface, 0x00, 0x00, 0x00, 0x00 ) );
 // DFB_CHECK (wsurface->FillRectangle (wsurface, 0, 0, w, h));

  //Clear the screen by filling a rectangle with the size of the surface.
  //Default color is black, default drawing flags are DSDRAW_NOFX.
}


void pxContext::setMatrix(pxMatrix4f& m)
{
  //  glUniformMatrix4fv(u_matrix, 1, GL_FALSE, m.data());
}


void pxContext::setAlpha(float a)
{
 // DFB_CHECK( window->SetOpacity(window, a * 255.0) );
}


pxTextureRef pxContext::createContextSurface(int width, int height)
{
  printf("\nINFO ####################### createContextSurface( %d,%d ) ", width, height);

//  pxFBOTexture* texture = new pxFBOTexture();
//  texture->createTexture(width, height);

//  return texture;

  return pxTextureRef();
}


pxError pxContext::updateContextSurface(pxTextureRef texture, int width, int height)
{
  if (texture.getPtr() == NULL)
  {
    return PX_FAIL;
  }
  
  return texture->resizeTexture(width, height);
}

pxTextureRef pxContext::getCurrentRenderSurface()
{
  return currentRenderSurface;
}


pxError pxContext::setRenderSurface(pxTextureRef texture)
{
  if(texture.getPtr() == NULL)
  {
    rtLogError("cannot setRenderSurface() on context surface because surface is NULL");
    return PX_FAIL;
  }

  printf("\nINFO ####################### setRenderSurface( ) ");

  if (texture.getPtr() != NULL)
  {
    //wsurface = texture->surface;
  }

  //  if (contextSurface == NULL)
  //  {
  //    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  //    return PX_OK;
  //  }

  //  if ((contextSurface->framebuffer == 0) || (contextSurface->texture == 0))
  //  {
  //    rtLog("render surface is not initialized");
  //    return PX_NOTINITIALIZED;
  //  }

  //  glBindFramebuffer(GL_FRAMEBUFFER, contextSurface->framebuffer);
  //  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
  //                          GL_TEXTURE_2D, contextSurface->texture, 0);

  //  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  //  {
  //    rtLog("error setting the render surface");
  //    return PX_FAIL;
  //  }

  //  glClearColor(0.0, 0.0, 0.0, 0.0);
  //  glClear(GL_COLOR_BUFFER_BIT);

  return PX_OK;
}


pxError pxContext::deleteContextSurface(pxTextureRef texture)
{
  if(texture.getPtr() == NULL)
  {
    rtLogError("cannot deleteContextSurface() on context surface because surface is NULL");
    return PX_FAIL;
  }

  //  IDirectFBSurface *surface = contextSurface->surface; //alias

  //  if(surface != 0)
  //  {
  //     surface->Release(surface);
  //  }

  //  if (contextSurface->framebuffer != 0)
  //  {
  //    glDeleteFramebuffers(1, &contextSurface->framebuffer);
  //    contextSurface->framebuffer = 0;
  //  }
  //  if (contextSurface->texture != 0)
  //  {
  //    glDeleteTextures(1, &contextSurface->texture);
  //    contextSurface->texture = 0;
  //  }
  return PX_OK;
}

void pxContext::drawRect(float w, float h, float lineWidth, float* fillColor, float* lineColor)
{
  if(wsurface == 0)
  {
    rtLogError("cannot drawRect on context surface because surface is NULL");
    return;
  }

  printf("\nINFO ####################### drawRect( ) ");

  DFB_CHECK (wsurface->SetColor(wsurface, fillColor[0] * 255.0,
             fillColor[1] * 255.0,
             fillColor[2] * 255.0,
             fillColor[3] * 255.0));

  float half = lineWidth/2;

  drawRect2(half, half, w-lineWidth, h-lineWidth);

  if (lineWidth > 0)
  {
    DFB_CHECK (wsurface->SetColor(wsurface, lineColor[0] * 255.0,
              lineColor[1] * 255.0,
              lineColor[2] * 255.0,
              lineColor[3] * 255.0));

    drawRectOutline(0, 0, w, h, lineWidth);
  }
}


void pxContext::drawDiagRect(float x, float y, float w, float h, float* color)
{
  if (!mShowOutlines) return;

  const float verts[4][2] =
  {
    {  x,   y   },
    {  x+w, y   },
    {  x+w, y+h },
    {  x,   y+h },
  };

  {
    //    glUniform4fv(u_color, 1, color);
    //    glUniform1f(u_alphatexture, 0.0);
    //    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    //    glEnableVertexAttribArray(attr_pos);
    //    glDrawArrays(GL_LINE_LOOP, 0, 4);
    //    glDisableVertexAttribArray(attr_pos);
  }
}

void pxContext::drawDiagLine(float x1, float y1, float x2, float y2, float* color)
{
  if (!mShowOutlines) return;
  const float verts[4][2] =
  {
    { x1, y1 },
    { x2, y2 },
  };

  {
    //    glUniform4fv(u_color, 1, color);
    //    glUniform1f(u_alphatexture, 0.0);
    //    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    //    glEnableVertexAttribArray(attr_pos);
    //    glDrawArrays(GL_LINES, 0, 2);
    //    glDisableVertexAttribArray(attr_pos);
  }
}


pxTextureRef pxContext::createTexture(float w, float h, float iw, float ih, void* buffer)
{
  pxTextureAlpha* alphaTexture = new pxTextureAlpha(w,h,iw,ih,buffer);
  return alphaTexture;
}

pxTextureRef pxContext::createTexture(pxOffscreen& o)
{
  printf("\nINFO ####################### createTexture( pxOffscreen ) ");

  pxTextureOffscreen* offscreenTexture = new pxTextureOffscreen(o);
  return offscreenTexture;
}

void pxContext::drawImage9(float w, float h, float x1, float y1,
                           float x2, float y2, pxOffscreen& o)
{
  drawImage92(0, 0, w, h, x1, y1, x2, y2, o);
}

void pxContext::drawImage(float w, float h, pxOffscreen& o,
                          pxStretch xStretch, pxStretch yStretch)
{
  printf("\nINFO ####################### drawImage( pxOffscreen ) ");

  drawImage2(0, 0, w, h, o, xStretch, yStretch);
}

void pxContext::drawImage(float x, float y, float w, float h, pxTextureRef t,
                          pxStretch xStretch, pxStretch yStretch)
{
//  printf("\nINFO ####################### drawImage( pxTextureRef - no mask) ");

  static pxTextureRef nullMaskRef;
  drawImageTexture(x, y, w, h, t, nullMaskRef, xStretch, yStretch);
}

void pxContext::drawImage(float x, float y, float w, float h, pxTextureRef t, pxTextureRef mask,
                          pxStretch xStretch, pxStretch yStretch, float* color /*= NULL*/)
{
//  printf("\nINFO ####################### drawImage( pxTextureRef: %s ) ", (t.getPtr() == NULL) ? "NULL" : "");

  color = color;

  drawImageTexture(x, y, w, h, t, mask, xStretch, yStretch);
}


void pxContext::drawImageAlpha(float x, float y, float w, float h, int bw, int bh, void* buffer, float* color)
{
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//  DFBRectangle rcSrc;

//  rcSrc.x = 0;
//  rcSrc.y = 0;
//  rcSrc.w = boundDesc.width;
//  rcSrc.h = boundDesc.height;

//  DFBRectangle rcDst;

//  rcDst.x = x;
//  rcDst.y = y;
//  rcDst.w = w;
//  rcDst.h = h;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 // DFB_CHECK(wsurface->StretchBlit(wsurface, boundSurface, &rcSrc, &rcDst));


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  //printf("\nINFO ####################### drawImageAlpha( x: %f, y: %f, w: %f, h: %f, bw: %f, bh %f, buffer: %p, color: ...) ", x,y,w,h,bw,bh, buffer);

  //    glActiveTexture(GL_TEXTURE1);

  //    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //    glTexImage2D(
  //      GL_TEXTURE_2D,
  //      0,
  //      GL_ALPHA,
  //      bw,
  //      bh,
  //      0,
  //      GL_ALPHA,
  //      GL_UNSIGNED_BYTE,
  //      buffer
  //    );

  //    glUniform1i(u_texture, 1);
  //    glUniform1f(u_alphatexture, 2.0);

  //    const float verts[4][2] =
  //    {
  //      { x,y },
  //      {  x+w, y },
  //      {  x,  y+h },
  //      {  x+w, y+h }
  //    };

  //    const float uv[4][2] =
  //    {
  //      { 0, 0 },
  //      { 1, 0 },
  //      { 0, 1 },
  //      { 1, 1 }
  //    };

  //    {
  //      glUniform4fv(u_color, 1, color);
  //      glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
  //      glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
  //      glEnableVertexAttribArray(attr_pos);
  //      glEnableVertexAttribArray(attr_uv);

  //      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  //      glDisableVertexAttribArray(attr_pos);
  //      glDisableVertexAttribArray(attr_uv);
  //    }
}

