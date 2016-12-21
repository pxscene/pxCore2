#include "rtDefs.h"
#include "rtLog.h"
#include "pxContext.h"
#include "rtNode.h"
#include "pxUtil.h"
#include "rtThreadTask.h"
#include "rtThreadPool.h"
#include "rtThreadQueue.h"
#include "rtMutex.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#endif //PX_PLATFORM_WAYLAND_EGL
#endif

#define PX_TEXTURE_MIN_FILTER GL_LINEAR
#define PX_TEXTURE_MAG_FILTER GL_LINEAR


////////////////////////////////////////////////////////////////
//
// Debug macros...

// NOTE:  Comment out these defines for 'normal' operation.
//
// #define DEBUG_SKIP_RECT
// #define DEBUG_SKIP_IMAGE
// #define DEBUG_SKIP_IMAGE9
// #define DEBUG_SKIP_MATRIX    // skip setMatrix()
// #define DEBUG_SKIP_APPLY     // skip applyMatrix()

// #define DEBUG_SKIP_DIAG_RECT
// #define DEBUG_SKIP_DIAG_LINE

// #define DEBUG_SKIP_FLIPPING
// #define DEBUG_SKIP_BLIT
// #define DEBUG_SKIP_ALPHA_BLEND

// #define DEBUG_SKIP_CLEAR  // pseudo color

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

pxContextSurfaceNativeDesc defaultContextSurface;
pxContextSurfaceNativeDesc* currentContextSurface = &defaultContextSurface;

pxContextFramebufferRef defaultFramebuffer(new pxContextFramebuffer());
pxContextFramebufferRef currentFramebuffer = defaultFramebuffer;

extern rtNode script;
extern pxContext context;
rtThreadQueue gUIThreadQueue;

enum pxCurrentGLProgram { PROGRAM_UNKNOWN = 0, PROGRAM_SOLID_SHADER,  PROGRAM_A_TEXTURE_SHADER, PROGRAM_TEXTURE_SHADER,
    PROGRAM_TEXTURE_MASKED_SHADER};

pxCurrentGLProgram currentGLProgram = PROGRAM_UNKNOWN;

// TODO get rid of this global crap

static int gResW, gResH;
static pxMatrix4f gMatrix;
static float gAlpha = 1.0;

// assume premultiplied
static const char *fSolidShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;"
#endif
  "uniform float u_alpha;"
  "uniform vec4 a_color;"
  "void main()"
  "{"
  "  gl_FragColor = a_color*u_alpha;"
  "}";

// assume premultiplied
static const char *fTextureShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;"
#endif
  "uniform sampler2D s_texture;"
  "uniform float u_alpha;"
  "varying vec2 v_uv;"
  "void main()"
  "{"
  "  gl_FragColor = texture2D(s_texture, v_uv) * u_alpha;"
  "}";

// assume premultiplied
static const char *fTextureMaskedShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;"
#endif
  "uniform sampler2D s_texture;"
  "uniform sampler2D s_mask;"
  "uniform float u_alpha;"
  "varying vec2 v_uv;"
  "void main()"
  "{"
  "  float a = u_alpha * texture2D(s_mask, v_uv).a;"
  "  gl_FragColor = texture2D(s_texture, v_uv) * a;"
  "}";

// assume premultiplied
static const char *fATextureShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;"
#endif
  "uniform sampler2D s_texture;"
  "uniform float u_alpha;"
  "uniform vec4 a_color;"
  "varying vec2 v_uv;"
  "void main()"
  "{"
  "  float a = u_alpha * texture2D(s_texture, v_uv).a;"
  "  gl_FragColor = a_color*a;"
  "}";

static const char *vShaderText =
  "uniform vec2 u_resolution;"
  "uniform mat4 amymatrix;"
  "attribute vec2 pos;"
  "attribute vec2 uv;"
  "varying vec2 v_uv;"
  "void main()"
  "{"
  // map from "pixel coordinates"
  "  vec4 p = amymatrix * vec4(pos, 0, 1);"
  "  vec4 zeroToOne = p / vec4(u_resolution, u_resolution.x, 1);"
  "  vec4 zeroToTwo = zeroToOne * vec4(2.0, 2.0, 1, 1);"
  "  vec4 clipSpace = zeroToTwo - vec4(1.0, 1.0, 0, 0);"
  "  clipSpace.w = 1.0+clipSpace.z;"
  "  gl_Position =  clipSpace * vec4(1, -1, 1, 1);"
  "  v_uv = uv;"
  "}";


//====================================================================================================================================================================================

inline void premultiply(float* d, const float* s)
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
  pxFBOTexture() : mWidth(0), mHeight(0), mFramebufferId(0), mTextureId(0), mBindTexture(true)
  {
    mTextureType = PX_TEXTURE_FRAME_BUFFER;
  }

  ~pxFBOTexture() { deleteTexture(); }

  void createTexture(int w, int h)
  {
    if (mFramebufferId != 0 && mTextureId != 0)
    {
      deleteTexture();
    }

    mWidth  = w;
    mHeight = h;

    glGenFramebuffers(1, &mFramebufferId);
    glGenTextures(1, &mTextureId);

    glBindTexture(GL_TEXTURE_2D, mTextureId); TRACK_TEX_CALLS();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 mWidth, mHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    context.adjustCurrentTextureMemorySize(mWidth*mHeight*4);
    mBindTexture = true;
  }

  pxError resizeTexture(int w, int h)
  {
    if (mWidth != w || mHeight != h ||
        mFramebufferId == 0 || mTextureId == 0)
    {
      createTexture(w, h);
      return PX_OK;
    }

    // TODO crashing in glTexSubImage2d in osx...
    #ifdef __APPLE__
    return PX_OK;
    #endif

    //TODO - remove commented out section
    /*glBindTexture(GL_TEXTURE_2D, mTextureId);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                 w, h, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
    return PX_OK;
  }

  virtual pxError deleteTexture()
  {
    if (mFramebufferId!= 0)
    {
      glDeleteFramebuffers(1, &mFramebufferId);
      mFramebufferId = 0;
    }

    if (mTextureId != 0)
    {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
      context.adjustCurrentTextureMemorySize(-1*mWidth*mHeight*4);
    }

    return PX_OK;
  }

  virtual unsigned int getNativeId()
  {
    return mTextureId;
  }

  virtual pxError prepareForRendering()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);   TRACK_FBO_CALLS();
    if (mBindTexture)
    {
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
      mBindTexture = false;
    }
    //glActiveTexture(GL_TEXTURE3);
    //glBindTexture(GL_TEXTURE_2D, mTextureId);
    glViewport ( 0, 0, mWidth, mHeight);
    gResW = mWidth;
    gResH = mHeight;

    return PX_OK;
  }

  // TODO get rid of pxError
  // TODO get rid of bindTexture() and bindTextureAsMask()
  virtual pxError bindGLTexture(int tLoc)
  {
    if (mFramebufferId == 0 || mTextureId == 0)
      return PX_NOTINITIALIZED;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTextureId);  TRACK_TEX_CALLS();
    glUniform1i(tLoc,1);

    return PX_OK;
  }

  virtual pxError bindGLTextureAsMask(int mLoc)
  {
    if (mFramebufferId == 0 || mTextureId == 0)
    {
      return PX_NOTINITIALIZED;
    }

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mTextureId);   TRACK_TEX_CALLS();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glUniform1i(mLoc, 2);
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

  virtual int width() { return mWidth; }
  virtual int height() { return mHeight; }

private:
  int mWidth;
  int mHeight;
  GLuint mFramebufferId;
  GLuint mTextureId;
  bool mBindTexture;

};// CLASS - pxFBOTexture


//====================================================================================================================================================================================

class pxTextureNone : public pxTexture
{
public:
  pxTextureNone() {}

  virtual int width()                                 { return 0;}
  virtual int height()                                { return 0;}
  virtual pxError deleteTexture()                     { return PX_FAIL; }
  virtual pxError resizeTexture(int /*w*/, int /*h*/) { return PX_FAIL; }
  virtual pxError getOffscreen(pxOffscreen& /*o*/)    { return PX_FAIL; }
  virtual pxError bindGLTexture(int /*tLoc*/)         { return PX_FAIL; }
  virtual pxError bindGLTextureAsMask(int /*mLoc*/)   { return PX_FAIL; }

};// CLASS - pxTextureNone

//====================================================================================================================================================================================

class pxTextureOffscreen;

struct DecodeImageData
{
    DecodeImageData(pxTextureRef t, pxOffscreen* o) : textureOffscreen(t), offscreen(o)
    {
    }
    pxTextureRef textureOffscreen;
    pxOffscreen* offscreen;

};

void onDecodeComplete(void* context, void* data)
{
  DecodeImageData* imageData = (DecodeImageData*)context;
  pxOffscreen* decodedOffscreen = (pxOffscreen*)data;
  if (imageData != NULL && decodedOffscreen != NULL)
  {
    pxTextureRef texture = imageData->textureOffscreen;
    if (texture.getPtr() != NULL)
    {
      texture->createTexture(*decodedOffscreen);
    }
  }

  if (decodedOffscreen != NULL)
  {
    delete decodedOffscreen;
    decodedOffscreen = NULL;
    data = NULL;
  }

  if (imageData != NULL)
  {
    delete imageData;
    imageData = NULL;
  }
}

void decodeTextureData(void* data)
{
  if (data != NULL)
  {
    DecodeImageData* imageData = (DecodeImageData*)data;
    pxOffscreen* offscreen = imageData->offscreen;
    if (offscreen != NULL)
    {
      char *compressedImageData = NULL;
      size_t compressedImageDataSize = 0;
      offscreen->compressedDataWeakReference(compressedImageData, compressedImageDataSize);
      pxOffscreen *decodedOffscreen = new pxOffscreen();
      pxLoadImage(compressedImageData, compressedImageDataSize, *decodedOffscreen);
      gUIThreadQueue.addTask(onDecodeComplete, data, decodedOffscreen);
    }
    else
    {
      gUIThreadQueue.addTask(onDecodeComplete, data, NULL);
    }
  }
}

void onOffscreenCleanupComplete(void* context, void*)
{
  DecodeImageData* imageData = (DecodeImageData*)context;
  if (imageData != NULL)
  {
    delete imageData;
    imageData = NULL;
  }
}

void cleanupOffscreen(void* data)
{
  if (data != NULL)
  {
    DecodeImageData* imageData = (DecodeImageData*)data;
    if (data != NULL && imageData->textureOffscreen.getPtr() != NULL)
    {
      imageData->textureOffscreen->freeOffscreenData();
      gUIThreadQueue.addTask(onOffscreenCleanupComplete, data, NULL);
    }
    else
    {
      gUIThreadQueue.addTask(onOffscreenCleanupComplete, data, NULL);
    }
  }
}

class pxTextureOffscreen : public pxTexture
{
public:
  pxTextureOffscreen() : mOffscreen(), mInitialized(false),
                         mTextureUploaded(false), mTextureDataAvailable(false),
                         mLoadTextureRequested(false), mWidth(0), mHeight(0), mOffscreenMutex(),
                         mFreeOffscreenDataRequested(false)
  {
    mTextureType = PX_TEXTURE_OFFSCREEN;
  }

  pxTextureOffscreen(pxOffscreen& o) : mOffscreen(), mInitialized(false),
                                       mTextureUploaded(false), mTextureDataAvailable(false),
                                       mLoadTextureRequested(false), mWidth(0), mHeight(0), mOffscreenMutex(),
                                       mFreeOffscreenDataRequested(false)
  {
    mTextureType = PX_TEXTURE_OFFSCREEN;
    createTexture(o);
  }

  ~pxTextureOffscreen() { deleteTexture(); };

  virtual pxError createTexture(pxOffscreen& o)
  {
    mOffscreenMutex.lock();
#ifdef ENABLE_MAX_TEXTURE_SIZE
    int verticalScale = 1;
    int horizontalScale = 1;
    int srcTextureWidth = o.width();
    int srcTextureHeight = o.height();
    int newTextureWidth = srcTextureWidth;
    int newTextureHeight = srcTextureHeight;
    if ( ((srcTextureWidth > MAX_TEXTURE_WIDTH) || (srcTextureHeight > MAX_TEXTURE_HEIGHT)))
    {
      while (newTextureWidth > MAX_TEXTURE_WIDTH)
      {
        horizontalScale <<= 1;
        newTextureWidth >>= 1;
      }
      while (newTextureHeight > MAX_TEXTURE_HEIGHT)
      {
        verticalScale <<= 1;
        newTextureHeight >>= 1;
      }
    }
    mWidth = srcTextureWidth;
    mHeight = srcTextureHeight;
    if ( (horizontalScale > 1) || (verticalScale > 1 ) )
    {
       mOffscreen.init(newTextureWidth, newTextureHeight);
       mOffscreen.setUpsideDown(true);
       int y = 0;
       for (int j = 0; j < srcTextureHeight-1; j += verticalScale, y++ )
       {
          int x = 0;
          for (int k = 0; k < srcTextureWidth-1; k += horizontalScale, x++)
          {
             o.blit(mOffscreen, x, y, 1,1,k,j);
          }
       }
    }
    else
    {
      mOffscreen.init(o.width(), o.height());
      // Flip the image data here so we match GL FBO layout
      mOffscreen.setUpsideDown(true);
      o.blit(mOffscreen);
    }
#else
    mOffscreen.init(o.width(), o.height());
    // Flip the image data here so we match GL FBO layout
    mOffscreen.setUpsideDown(true);
    o.blit(mOffscreen);
    mWidth = mOffscreen.width();
    mHeight = mOffscreen.height();
#endif //ENABLE_MAX_TEXTURE_SIZE

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

    mOffscreen.transferCompressedDataFrom(o);
    mFreeOffscreenDataRequested = false;
    mOffscreenMutex.unlock();

    mTextureDataAvailable = true;
    mLoadTextureRequested = false;
    mInitialized = true;

    return PX_OK;
  }

  virtual pxError deleteTexture()
  {
    rtLogDebug("pxTextureOffscreen::deleteTexture()");

    unloadTextureData();

    mOffscreen.freeCompressedData();
    mTextureDataAvailable = false;
    mInitialized = false;
    return PX_OK;
  }

  virtual pxError loadTextureData()
  {
    if (!mLoadTextureRequested && mTextureDataAvailable)
    {
      rtThreadPool *mainThreadPool = rtThreadPool::globalInstance();
      DecodeImageData *decodeImageData = new DecodeImageData(this, &mOffscreen);
      rtThreadTask *task = new rtThreadTask(decodeTextureData, decodeImageData, "");
      mainThreadPool->executeTask(task);
      mLoadTextureRequested = true;
    }

    return PX_OK;
  }

  virtual pxError unloadTextureData()
  {
    if (mInitialized)
    {
      if (mTextureName)
      {
        glDeleteTextures(1, &mTextureName);
        context.adjustCurrentTextureMemorySize(-1 * mWidth * mHeight * 4);
      }

      mTextureName = 0;
      mInitialized = false;
      mTextureUploaded = false;
      mOffscreenMutex.lock();
      mOffscreen.term();
      mFreeOffscreenDataRequested = false;
      mOffscreenMutex.unlock();
    }
    return PX_OK;
  }

  virtual pxError freeOffscreenData()
  {
    mOffscreenMutex.lock();
    if (mFreeOffscreenDataRequested)
    {
      rtLogDebug("freeing offscreen data");
      mOffscreen.term();
    }
    mFreeOffscreenDataRequested = false;
    mOffscreenMutex.unlock();
    return PX_OK;
  }

  virtual pxError bindGLTexture(int tLoc)
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }


    glActiveTexture(GL_TEXTURE1);


// TODO would be nice to do the upload in createTexture but right now it's getting called on wrong thread
    if (!mTextureUploaded)
    {
      if (!context.isTextureSpaceAvailable(this))
      {
        rtLogError("not enough texture memory remaining to create texture");
        mInitialized = false;
        freeOffscreenDataInBackground();
        return PX_FAIL;
      }
      glGenTextures(1, &mTextureName);
      glBindTexture(GL_TEXTURE_2D, mTextureName);   TRACK_TEX_CALLS();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                   mOffscreen.width(), mOffscreen.height(), 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, mOffscreen.base());
      mTextureUploaded = true;
      context.adjustCurrentTextureMemorySize(mOffscreen.width()*mOffscreen.height()*4);
      //free up unneeded offscreen memory
      freeOffscreenDataInBackground();
    }
    else
    {
      glBindTexture(GL_TEXTURE_2D, mTextureName);   TRACK_TEX_CALLS();
    }

    glUniform1i(tLoc, 1);
    return PX_OK;
  }

  virtual pxError bindGLTextureAsMask(int mLoc)
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }

    glActiveTexture(GL_TEXTURE2);

    if (!mTextureUploaded)
    {
      if (!context.isTextureSpaceAvailable(this))
      {
        rtLogError("not enough texture memory remaining to create texture");
        mInitialized = false;
        freeOffscreenDataInBackground();
        return PX_FAIL;
      }
      glGenTextures(1, &mTextureName);
      glBindTexture(GL_TEXTURE_2D, mTextureName);   TRACK_TEX_CALLS();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                   mOffscreen.width(), mOffscreen.height(), 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, mOffscreen.base());
      mTextureUploaded = true;
      context.adjustCurrentTextureMemorySize(mOffscreen.width()*mOffscreen.height()*4);
      
      //free up unneeded offscreen memory
      freeOffscreenDataInBackground();
    }
    else
    {
      glBindTexture(GL_TEXTURE_2D, mTextureName);   TRACK_TEX_CALLS();
    }

    glUniform1i(mLoc, 2);

    return PX_OK;
  }

  virtual pxError getOffscreen(pxOffscreen& o)
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }

    char* compressedImageData = NULL;
    size_t compressedImageDataSize = 0;
    mOffscreen.compressedDataWeakReference(compressedImageData, compressedImageDataSize);
    if (compressedImageData != NULL)
    {
      pxLoadImage(compressedImageData, compressedImageDataSize, o);
    }

    return PX_OK;
  }

  virtual int width()  { return mWidth;  }
  virtual int height() { return mHeight; }

private:

  void freeOffscreenDataInBackground()
  {
    mOffscreenMutex.lock();
    mFreeOffscreenDataRequested = true;
    mOffscreenMutex.unlock();
    rtLogDebug("request to free offscreen data");
    rtThreadPool *mainThreadPool = rtThreadPool::globalInstance();
    DecodeImageData *imageData = new DecodeImageData(this, NULL);
    rtThreadTask *task = new rtThreadTask(cleanupOffscreen, imageData, "");
    mainThreadPool->executeTask(task);
  }

  pxOffscreen mOffscreen;
  
  bool mInitialized;
  GLuint mTextureName;
  bool mTextureUploaded;
  bool mTextureDataAvailable;
  bool mLoadTextureRequested;
  int mWidth;
  int mHeight;
  rtMutex mOffscreenMutex;
  bool mFreeOffscreenDataRequested;

}; // CLASS - pxTextureOffscreen


//====================================================================================================================================================================================

class pxTextureAlpha : public pxTexture
{
public:
  pxTextureAlpha() : mDrawWidth(0.0), mDrawHeight (0.0), mImageWidth(0.0),
                     mImageHeight(0.0), mTextureId(0), mInitialized(false)
  {
    mTextureType = PX_TEXTURE_ALPHA;
  }

  pxTextureAlpha(float w, float h, float iw, float ih, void* buffer)
    : mDrawWidth(w),    mDrawHeight (h),
      mImageWidth(iw), mImageHeight(ih),
      mTextureId(0), mInitialized(false), mBuffer(NULL)
  {
    mTextureType = PX_TEXTURE_ALPHA;

    // copy the pixels
    int bitmapSize = ih*iw;
    mBuffer = malloc(bitmapSize);

    // TODO consider iw,ih as ints rather than floats...
    int32_t bw = (int32_t)iw;
    int32_t bh = (int32_t)ih;

    //memcpy(mBuffer, buffer, bitmapSize);
    // Flip here so that we match FBO layout...
    for (int32_t i = 0; i < bh; i++)
    {
      uint8_t *s = (uint8_t*)buffer+(bw*i);
      uint8_t *d = (uint8_t*)mBuffer+(bw*(bh-i-1));
      uint8_t *de = d+bw;
      while(d<de)
        *d++ = *s++;
    }

// TODO Moved this to bindTexture because of more pain from JS thread calls
//    createTexture(w, h, iw, ih);
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
    if (mTextureId != 0)
    {
      deleteTexture();
    }

    if(iw == 0 || ih == 0)
    {
      rtLogError("pxTextureAlpha::createTexture() - DIMENSIONLESS ");
      return; // DIMENSIONLESS
    }

    glGenTextures(1, &mTextureId);

    mDrawWidth   = w;
    mDrawHeight  = h;
    mImageWidth  = iw;
    mImageHeight = ih;

//    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTextureId);   TRACK_TEX_CALLS();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_ALPHA,
      iw,
      ih,
      0,
      GL_ALPHA,
      GL_UNSIGNED_BYTE,
      mBuffer
    );
    context.adjustCurrentTextureMemorySize(iw*ih);

    mInitialized = true;
  }

  virtual pxError deleteTexture()
  {
    if (mTextureId != 0)
    {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
      context.adjustCurrentTextureMemorySize(-1*mImageWidth*mImageHeight);
    }
    mInitialized = false;
    return PX_OK;
  }

  virtual pxError bindGLTexture(int tLoc)
  {
    // TODO Moved to here because of js threading issues
    if (!mInitialized) createTexture(mDrawWidth,mDrawHeight,mImageWidth,mImageHeight);
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTextureId);   TRACK_TEX_CALLS();
    glUniform1i(tLoc, 1);

    return PX_OK;
  }

  virtual pxError bindGLTextureAsMask(int mLoc)
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mTextureId);   TRACK_TEX_CALLS();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glUniform1i(mLoc, 2);

    return PX_OK;
  }

  virtual pxError getOffscreen(pxOffscreen& /*o*/)
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }
    return PX_FAIL;
  }

  virtual int width()  {return mDrawWidth;  }
  virtual int height() {return mDrawHeight; }

private:
  float mDrawWidth;
  float mDrawHeight;
  float mImageWidth;
  float mImageHeight;
  GLuint mTextureId;
  bool mInitialized;
  void* mBuffer;

}; // CLASS - pxTextureAlpha

//====================================================================================================================================================================================

static GLuint createShaderProgram(const char* vShaderTxt, const char* fShaderTxt)
{
  GLuint fragShader, vertShader, program = 0;
  GLint stat;

  fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragShader, 1, (const char **) &fShaderTxt, NULL);
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &stat);

  if (!stat)
  {
    rtLogError("Error: fragment shader did not compile: %d", glGetError());

    GLint maxLength = 0;
    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);

    //The maxLength includes the NULL character
    std::vector<char> errorLog(maxLength);
    glGetShaderInfoLog(fragShader, maxLength, &maxLength, &errorLog[0]);

    rtLogWarn("%s", &errorLog[0]);
    //Exit with failure.
    glDeleteShader(fragShader); //Don't leak the shader.

    //TODO get rid of exit
    exit(1);
  }

  vertShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertShader, 1, (const char **) &vShaderTxt, NULL);
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &stat);

  if (!stat)
  {
    rtLogError("vertex shader did not compile: %d", glGetError());
    exit(1);
  }

  program = glCreateProgram();
  glAttachShader(program, fragShader);
  glAttachShader(program, vertShader);

  return program;
}

void linkShaderProgram(GLuint program)
{
  GLint stat;

  glLinkProgram(program);  /* needed to put attribs into effect */
  glGetProgramiv(program, GL_LINK_STATUS, &stat);
  if (!stat)
  {
    char log[1000];
    GLsizei len;
    glGetProgramInfoLog(program, 1000, &len, log);
    rtLogError("faild to link:%s", log);
    // TODO purge all exit calls
    exit(1);
  }
}

//====================================================================================================================================================================================

class shaderProgram
{
public:
  virtual ~shaderProgram() {}
  virtual void init(const char* v, const char* f)
  {
    mProgram = createShaderProgram(v, f);

    prelink();
    linkShaderProgram(mProgram);
    postlink();
  }

  int getUniformLocation(const char* name)
  {
    int l = glGetUniformLocation(mProgram, name);
    if (l == -1)
      rtLogError("Shader does not define uniform %s.\n", name);
    return l;
  }

  void use()
  {
    currentGLProgram = PROGRAM_UNKNOWN;
    glUseProgram(mProgram);
  }

protected:
  // Override to do uniform lookups
  virtual void prelink() {}
  virtual void postlink() {}

  GLuint mProgram;

}; // CLASS - shaderProgram

//====================================================================================================================================================================================

class solidShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    mPosLoc = 0;
    mUVLoc = 1;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc, "uv");
  }

  virtual void postlink()
  {
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc = getUniformLocation("amymatrix");
    mColorLoc = getUniformLocation("a_color");
    mAlphaLoc = getUniformLocation("u_alpha");
  }

public:
  pxError draw(int resW, int resH, float* matrix, float alpha,
            GLenum mode,
            const void* pos,
            int count,
            const float* color)
  {
    if (currentGLProgram != PROGRAM_SOLID_SHADER)
    {
      use();
      currentGLProgram = PROGRAM_SOLID_SHADER;
    }
    glUniform2f(mResolutionLoc, resW, resH);
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1f(mAlphaLoc, alpha);
    glUniform4fv(mColorLoc, 1, color);

    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glEnableVertexAttribArray(mPosLoc);
    glDrawArrays(mode, 0, count);  ;
    glDisableVertexAttribArray(mPosLoc);

    return PX_OK;
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mColorLoc;
  GLint mAlphaLoc;

}; //CLASS - solidShaderProgram

solidShaderProgram *gSolidShader = NULL;

//====================================================================================================================================================================================

class aTextureShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    mPosLoc = 0;
    mUVLoc = 1;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc, "uv");
  }

  virtual void postlink()
  {
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc = getUniformLocation("amymatrix");
    mColorLoc = getUniformLocation("a_color");
    mAlphaLoc = getUniformLocation("u_alpha");
    mTextureLoc = getUniformLocation("s_texture");
  }

public:
  pxError draw(int resW, int resH, float* matrix, float alpha,
            int count,
            const void* pos,
            const void* uv,
            pxTextureRef texture,
            const float* color)
  {
    if (currentGLProgram != PROGRAM_A_TEXTURE_SHADER)
    {
      use();
      currentGLProgram = PROGRAM_A_TEXTURE_SHADER;
    }
    glUniform2f(mResolutionLoc, resW, resH);
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1f(mAlphaLoc, alpha);
    glUniform4fv(mColorLoc, 1, color);

    if (texture->bindGLTexture(mTextureLoc) != PX_OK)
    {
      return PX_FAIL;
    }

    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glVertexAttribPointer(mUVLoc, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(mPosLoc);
    glEnableVertexAttribArray(mUVLoc);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, count);  TRACK_DRAW_CALLS();
    glDisableVertexAttribArray(mPosLoc);
    glDisableVertexAttribArray(mUVLoc);

    return PX_OK;
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mColorLoc;
  GLint mAlphaLoc;

  GLint mTextureLoc;

}; //CLASS - aTextureShaderProgram

aTextureShaderProgram *gATextureShader = NULL;

//====================================================================================================================================================================================

class textureShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    mPosLoc = 0;
    mUVLoc = 1;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc, "uv");
  }

  virtual void postlink()
  {
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc = getUniformLocation("amymatrix");
    mAlphaLoc = getUniformLocation("u_alpha");
    mTextureLoc = getUniformLocation("s_texture");
  }

public:
  pxError draw(int resW, int resH, float* matrix, float alpha,
            int count,
            const void* pos, const void* uv,
            pxTextureRef texture,
            int32_t stretchX, int32_t stretchY)
  {
    if (currentGLProgram != PROGRAM_TEXTURE_SHADER)
    {
      use();
      currentGLProgram = PROGRAM_TEXTURE_SHADER;
    }
    glUniform2f(mResolutionLoc, resW, resH);
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1f(mAlphaLoc, alpha);

    if (texture->bindGLTexture(mTextureLoc) != PX_OK)
    {
      return PX_FAIL;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		    (stretchX==pxConstantsStretch::REPEAT)?GL_REPEAT:GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		    (stretchY==pxConstantsStretch::REPEAT)?GL_REPEAT:GL_CLAMP_TO_EDGE);

    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glVertexAttribPointer(mUVLoc, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(mPosLoc);
    glEnableVertexAttribArray(mUVLoc);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, count);  TRACK_DRAW_CALLS();
    glDisableVertexAttribArray(mPosLoc);
    glDisableVertexAttribArray(mUVLoc);

    return PX_OK;
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mAlphaLoc;

  GLint mTextureLoc;

}; //CLASS - textureShaderProgram

textureShaderProgram *gTextureShader = NULL;

//====================================================================================================================================================================================

class textureMaskedShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    mPosLoc = 0;
    mUVLoc = 1;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc, "uv");
  }

  virtual void postlink()
  {
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc = getUniformLocation("amymatrix");
    mAlphaLoc = getUniformLocation("u_alpha");
    mTextureLoc = getUniformLocation("s_texture");
    mMaskLoc = getUniformLocation("s_mask");
  }

public:
  pxError draw(int resW, int resH, float* matrix, float alpha,
            int count,
            const void* pos,
            const void* uv,
            pxTextureRef texture,
            pxTextureRef mask)
  {
    if (currentGLProgram != PROGRAM_TEXTURE_MASKED_SHADER)
    {
      use();
      currentGLProgram = PROGRAM_TEXTURE_MASKED_SHADER;
    }
    glUniform2f(mResolutionLoc, resW, resH);
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1f(mAlphaLoc, alpha);

    if (texture->bindGLTexture(mTextureLoc) != PX_OK)
    {
      return PX_FAIL;
    }

    if (mask.getPtr() != NULL)
    {
      if (mask->bindGLTextureAsMask(mMaskLoc) != PX_OK)
      {
        return PX_FAIL;
      }
    }


    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glVertexAttribPointer(mUVLoc, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(mPosLoc);
    glEnableVertexAttribArray(mUVLoc);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, count);  TRACK_DRAW_CALLS();
    glDisableVertexAttribArray(mPosLoc);
    glDisableVertexAttribArray(mUVLoc);

    return PX_OK;
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mAlphaLoc;

  GLint mTextureLoc;
  GLint mMaskLoc;

}; //CLASS - textureMaskedShaderProgram

textureMaskedShaderProgram *gTextureMaskedShader = NULL;

//====================================================================================================================================================================================

static void drawRect2(GLfloat x, GLfloat y, GLfloat w, GLfloat h, const float* c)
{
  // args are tested at call site...

  const float verts[4][2] =
  {
    { x  , y   },
    { x+w, y   },
    { x  , y+h },
    { x+w, y+h }
  };

  float colorPM[4];
  premultiply(colorPM,c);

  gSolidShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_TRIANGLE_STRIP,verts,4,colorPM);
}


static void drawRectOutline(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat lw, const float* c)
{
  // args are tested at call site...

  float ox1  = x;
  float ix1  = x+lw;
  float ox2  = x+w;
  float ix2  = x+w-lw;
  float oy1  = y;
  float iy1  = y+lw;
  float oy2  = y+h;
  float iy2  = y+h-lw;

  const GLfloat verts[10][2] =
  {
    { ox1,oy1 },
    { ix1,iy1 },
    { ox2,oy1 },
    { ix2,iy1 },
    { ox2,oy2 },
    { ix2,iy2 },
    { ox1,oy2 },
    { ix1,iy2 },
    { ox1,oy1 },
    { ix1,iy1 }
  };

  float colorPM[4];
  premultiply(colorPM,c);

  gSolidShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_TRIANGLE_STRIP,verts,10,colorPM);
}

static void drawImageTexture(float x, float y, float w, float h, pxTextureRef texture,
                             pxTextureRef mask, bool useTextureDimsAlways, float* color, // default: "color = BLACK"
                             pxConstantsStretch::constants xStretch,
                             pxConstantsStretch::constants yStretch)
{
  // args are tested at call site...

  float iw = texture->width();
  float ih = texture->height();

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
    tw = iw/w;
    break;
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
    th = ih/h;
    break;
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

  float firstTextureY  = th;
  float secondTextureY = tb;

  const float uv[4][2] =
  {
    { 0,  firstTextureY  },
    { tw, firstTextureY  },
    { 0,  secondTextureY },
    { tw, secondTextureY }
  };

  float colorPM[4];
  premultiply(colorPM,color);

  static float blackColor[4] = {0.0, 0.0, 0.0, 1.0};

  if (mask.getPtr() == NULL && texture->getType() != PX_TEXTURE_ALPHA)
  {
    if (gTextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,4,verts,uv,texture,xStretch,yStretch) != PX_OK)
    {
      drawRect2(0, 0, iw, ih, blackColor);
    }
  }
  else if (mask.getPtr() == NULL && texture->getType() == PX_TEXTURE_ALPHA)
  {
    if (gATextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,4,verts,uv,texture,colorPM) != PX_OK)
    {
      drawRect2(0, 0, iw, ih, blackColor);
    }
  }
  else if (mask.getPtr() != NULL)
  {
    if (gTextureMaskedShader->draw(gResW,gResH,gMatrix.data(),gAlpha,4,verts,uv,texture,mask) != PX_OK)
    {
      drawRect2(0, 0, iw, ih, blackColor);
    }
  }
  else
  {
    rtLogError("Unhandled case");
  }
}

static void drawImage92(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat x1, GLfloat y1, GLfloat x2,
                        GLfloat y2, pxTextureRef texture)
{
  // args are tested at call site...

  float ox1 = x;
  float ix1 = x+x1;
  float ix2 = x+w-x2;
  float ox2 = x+w;

  float oy1 = y;
  float iy1 = y+y1;
  float iy2 = y+h-y2;
  float oy2 = y+h;

  float w2 = texture->width();
  float h2 = texture->height();

  float ou1 = 0;
  float iu1 = x1/w2;
  float iu2 = (w2-x2)/w2;
  float ou2 = 1;

  float ov2 = 0;
  float iv2 = y1/h2;
  float iv1 = (h2-y2)/h2;
  float ov1 = 1;

#if 1 // sanitize values
  iu1 = pxClamp<float>(iu1, 0, 1);
  iu2 = pxClamp<float>(iu2, 0, 1);
  iv1 = pxClamp<float>(iv1, 0, 1);
  iv2 = pxClamp<float>(iv2, 0, 1);

  float tmin, tmax;

  tmin = pxMin<float>(iu1, iu2);
  tmax = pxMax<float>(iu1, iu2);
  iu1 = tmin;
  iu2 = tmax;

  tmin = pxMin<float>(iv1, iv2);
  tmax = pxMax<float>(iv1, iv2);
  iv1 = tmax;
  iv2 = tmin;

#endif

  const GLfloat verts[22][2] =
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

  const GLfloat uv[22][2] =
  {
    { ou1,ov1 },
    { iu1,ov1 },
    { ou1,iv1 },
    { iu1,iv1 },
    { ou1,iv2 },
    { iu1,iv2 },
    { ou1,ov2 },
    { iu1,ov2 },
    { iu2,ov2 },
    { iu1,iv2 },
    { iu2,iv2 },
    { iu1,iv1 },
    { iu2,iv1 },
    { iu1,ov1 },
    { iu2,ov1 },
    { ou2,ov1 },
    { iu2,iv1 },
    { ou2,iv1 },
    { iu2,iv2 },
    { ou2,iv2 },
    { iu2,ov2 },
    { ou2,ov2 }
  };

  gTextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,22,verts,uv,texture,pxConstantsStretch::NONE,pxConstantsStretch::NONE);
}

bool gContextInit = false;

pxContext::~pxContext()
{
  if (gSolidShader)
  {
    delete gSolidShader;
    gSolidShader = NULL;
  }

  if (gATextureShader)
  {
    delete gATextureShader;
    gATextureShader = NULL;
  }

  if (gTextureShader)
  {
    delete gTextureShader;
    gTextureShader = NULL;
  }

  if (gTextureMaskedShader)
  {
    delete gTextureMaskedShader;
    gTextureMaskedShader = NULL;
  }
}

void pxContext::init()
{
#if 0
  if (gContextInit)
    return;
  else
    gContextInit = true;
#endif

  glClearColor(0, 0, 0, 0);

  if (gSolidShader)
  {
    delete gSolidShader;
    gSolidShader = NULL;
  }

  if (gATextureShader)
  {
    delete gATextureShader;
    gATextureShader = NULL;
  }

  if (gTextureShader)
  {
    delete gTextureShader;
    gTextureShader = NULL;
  }

  if (gTextureMaskedShader)
  {
    delete gTextureMaskedShader;
    gTextureMaskedShader = NULL;
  }

  gSolidShader = new solidShaderProgram();
  gSolidShader->init(vShaderText,fSolidShaderText);

  gATextureShader = new aTextureShaderProgram();
  gATextureShader->init(vShaderText,fATextureShaderText);

  gTextureShader = new textureShaderProgram();
  gTextureShader->init(vShaderText,fTextureShaderText);

  gTextureMaskedShader = new textureMaskedShaderProgram();
  gTextureMaskedShader->init(vShaderText,fTextureMaskedShaderText);

  glEnable(GL_BLEND);

  // assume non-premultiplied for now...
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
// non-premultiplied
//  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//  glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//  glUseProgram(program);

//  gprogram = program;
  setTextureMemoryLimit(PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_IN_BYTES);
}

void pxContext::setSize(int w, int h)
{
  glViewport(0, 0, (GLint)w, (GLint)h);
  gResW = w;
  gResH = h;

  if (currentFramebuffer == defaultFramebuffer)
  {
    defaultContextSurface.width = w;
    defaultContextSurface.height = h;
    gResW = w;
    gResH = h;
  }
}

void pxContext::getSize(int& w, int& h)
{
   w = gResW;
   h = gResH;
}

void pxContext::clear(int /*w*/, int /*h*/)
{
  glClear(GL_COLOR_BUFFER_BIT);
}

void pxContext::clear(int /*w*/, int /*h*/, float *fillColor )
{
  float color[4];

  glGetFloatv( GL_COLOR_CLEAR_VALUE, color );
  glClearColor( fillColor[0], fillColor[1], fillColor[2], fillColor[3] );
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor( color[0], color[1], color[2], color[3] );
  currentFramebuffer->enableDirtyRectangles(false);
}

void pxContext::clear(int left, int top, int right, int bottom)
{
  glEnable(GL_SCISSOR_TEST); //todo - not set each frame

  currentFramebuffer->setDirtyRectangle(left, gResH-top-bottom, right, bottom);
  currentFramebuffer->enableDirtyRectangles(true);

  //map form screen to window coordinates
  glScissor(left, gResH-top-bottom, right, bottom);
  //glClear(GL_COLOR_BUFFER_BIT);
}

void pxContext::setMatrix(pxMatrix4f& m)
{
  gMatrix.multiply(m);
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

pxContextFramebufferRef pxContext::createFramebuffer(int width, int height)
{
  pxContextFramebuffer* fbo = new pxContextFramebuffer();
  pxFBOTexture* texture = new pxFBOTexture();

  texture->createTexture(width, height);

  fbo->setTexture(texture);

  return fbo;
}

pxError pxContext::updateFramebuffer(pxContextFramebufferRef fbo, int width, int height)
{
  if (fbo.getPtr() == NULL || fbo->getTexture().getPtr() == NULL)
  {
    return PX_FAIL;
  }

  return fbo->getTexture()->resizeTexture(width, height);
}

pxContextFramebufferRef pxContext::getCurrentFramebuffer()
{
  return currentFramebuffer;
}

pxError pxContext::setFramebuffer(pxContextFramebufferRef fbo)
{
  if (fbo.getPtr() == NULL || fbo->getTexture().getPtr() == NULL)
  {
    glViewport ( 0, 0, defaultContextSurface.width, defaultContextSurface.height);

    gResW = defaultContextSurface.width;
    gResH = defaultContextSurface.height;

    // TODO probably need to save off the original FBO handle rather than assuming zero
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  TRACK_FBO_CALLS();
    currentFramebuffer = defaultFramebuffer;

    pxContextState contextState;
    currentFramebuffer->currentState(contextState);

    gAlpha = contextState.alpha;
    gMatrix = contextState.matrix;

#ifdef PX_DIRTY_RECTANGLES
    if (currentFramebuffer->isDirtyRectanglesEnabled())
    {
      glEnable(GL_SCISSOR_TEST);
      pxRect dirtyRect = currentFramebuffer->dirtyRectangle();
      glScissor(dirtyRect.left(), dirtyRect.top(), dirtyRect.right(), dirtyRect.bottom());
    }
    else
    {
      glDisable(GL_SCISSOR_TEST);
    }
#endif //PX_DIRTY_RECTANGLES
    return PX_OK;
  }

  currentFramebuffer = fbo;
  pxContextState contextState;
  currentFramebuffer->currentState(contextState);
  gAlpha = contextState.alpha;
  gMatrix = contextState.matrix;

#ifdef PX_DIRTY_RECTANGLES
  if (currentFramebuffer->isDirtyRectanglesEnabled())
  {
    glEnable(GL_SCISSOR_TEST);
    pxRect dirtyRect = currentFramebuffer->dirtyRectangle();
    glScissor(dirtyRect.left(), dirtyRect.top(), dirtyRect.right(), dirtyRect.bottom());
  }
  else
  {
    glDisable(GL_SCISSOR_TEST);
  }
#endif //PX_DIRTY_RECTANGLES

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

void pxContext::enableDirtyRectangles(bool enable)
{
  currentFramebuffer->enableDirtyRectangles(enable);
  if (enable)
  {
    glEnable(GL_SCISSOR_TEST);
    pxRect dirtyRect = currentFramebuffer->dirtyRectangle();
    glScissor(dirtyRect.left(), dirtyRect.top(), dirtyRect.right(), dirtyRect.bottom());
  }
  else
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void pxContext::drawRect(float w, float h, float lineWidth, float* fillColor, float* lineColor)
{
#ifdef DEBUG_SKIP_RECT
#warning "DEBUG_SKIP_RECT enabled ... Skipping "
  return;
#endif

  // TRANSPARENT / DIMENSIONLESS 
  if(gAlpha == 0.0 || w <= 0.0 || h <= 0.0)
  {
   // printf("\n drawRect() - TRANSPARENT");
    return;
  }

  // COLORLESS
  if(fillColor == NULL && lineColor == NULL)
  {
    //rtLogError("cannot drawRect() on context surface because colors are NULL");
    return;
  }

  // Fill ...
  if(fillColor != NULL && fillColor[3] > 0.0) // with non-transparent color
  {
    float half = lineWidth/2;
    drawRect2(half, half, w-lineWidth, h-lineWidth, fillColor);
  }

  // Frame ...
  if(lineColor != NULL && lineColor[3] > 0.0 && lineWidth > 0) // with non-transparent color and non-zero stroke
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

  // TRANSPARENT / DIMENSIONLESS 
  if(gAlpha == 0.0 || w <= 0.0 || h <= 0.0)
  {
    return;
  }

  // TEXTURELESS
  if (texture.getPtr() == NULL)
  {
    return;
  }

  drawImage92(0, 0, w, h, x1, y1, x2, y2, texture);
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

  // TRANSPARENT / DIMENSIONLESS 
  if(gAlpha == 0.0 || w <= 0.0 || h <= 0.0)
  {
    return;
  }

  // TEXTURELESS
  if (t.getPtr() == NULL)
  {
    return;
  }

  float black[4] = {0,0,0,1};
  drawImageTexture(x, y, w, h, t, mask, useTextureDimsAlways,
                  color? color : black, stretchX, stretchY);
}

void pxContext::drawDiagRect(float x, float y, float w, float h, float* color)
{
#ifdef DEBUG_SKIP_DIAG_RECT
#warning "DEBUG_SKIP_DIAG_RECT enabled ... Skipping "
   return;
#endif

  if (!mShowOutlines) return;

  // TRANSPARENT / DIMENSIONLESS 
  if(gAlpha == 0.0 || w <= 0.0 || h <= 0.0)
  {
    rtLogError("cannot drawDiagRect() - width/height/gAlpha cannot be Zero.");
    return;
  }

  // COLORLESS
  if(color == NULL || color[3] == 0.0)
  {
    return; 
  }


  const float verts[4][2] =
  {
    { x  , y   },
    { x+w, y   },
    { x+w, y+h },
    { x  , y+h },
   };


  float colorPM[4];
  premultiply(colorPM,color);

  gSolidShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_LINE_LOOP,verts,4,colorPM);
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
    return; // TRANSPARENT
  }

  if(color == NULL || color[3] == 0.0)
  {
    return; // COLORLESS
  }

  const float verts[4][2] =
  {
    { x1, y1 },
    { x2, y2 },
   };

  float colorPM[4];
  premultiply(colorPM,color);

  gSolidShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_LINES,verts,2,colorPM);
}

pxTextureRef pxContext::createTexture()
{
  pxTextureNone* noneTexture = new pxTextureNone();
  return noneTexture;
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
  contextState.alpha = gAlpha;

  currentFramebuffer->pushState(contextState);
}

void pxContext::popState()
{
  pxContextState contextState;
  if (currentFramebuffer->popState(contextState) == PX_OK)
  {
    gAlpha = contextState.alpha;
    gMatrix = contextState.matrix;
  }
}

void pxContext::snapshot(pxOffscreen& o)
{
  o.init(gResW,gResH);
  glReadPixels(0,0,gResW,gResH,GL_RGBA,GL_UNSIGNED_BYTE,(void*)o.base());

  o.setUpsideDown(true);
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

void pxContext::adjustCurrentTextureMemorySize(int64_t changeInBytes)
{
  mCurrentTextureMemorySizeInBytes += changeInBytes;
  if (mCurrentTextureMemorySizeInBytes < 0)
  {
    mCurrentTextureMemorySizeInBytes = 0;
  }
#ifdef ENABLE_PX_SCENE_TEXTURE_USAGE_MONITORING
  if (changeInBytes > 0 && mCurrentTextureMemorySizeInBytes > mTextureMemoryLimitInBytes)
  {
    rtLogDebug("the texture size is too large: %" PRId64 ".  doing a garbage collect!!!\n", mCurrentTextureMemorySizeInBytes);
    script.garbageCollect();
  }
#endif // ENABLE_PX_SCENE_TEXTURE_USAGE_MONITORING
}

void pxContext::setTextureMemoryLimit(int64_t textureMemoryLimitInBytes)
{
  mTextureMemoryLimitInBytes = textureMemoryLimitInBytes;
}

bool pxContext::isTextureSpaceAvailable(pxTextureRef texture)
{
#ifdef ENABLE_PX_SCENE_TEXTURE_USAGE_MONITORING
  int textureSize = (texture->width()*texture->height()*4);
  if ((textureSize + mCurrentTextureMemorySizeInBytes) >
             (mTextureMemoryLimitInBytes  + PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_THRESHOLD_PADDING_IN_BYTES))
  {
    return false;
  }
  else
  {
    return true;
  }
#endif //ENABLE_PX_SCENE_TEXTURE_USAGE_MONITORING
  return true;
}

