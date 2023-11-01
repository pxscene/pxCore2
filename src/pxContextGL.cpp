/*

 pxCore Copyright 2005-2018 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// pxContextGL.cpp

#include "rtCore.h"
#include "rtLog.h"
#include "rtThreadTask.h"
#include "rtThreadPool.h"
#include "rtThreadQueue.h"
#include "rtMutex.h"
#include "rtScript.h"
#include "rtSettings.h"

#include "pxShaderUtils.h"

#include "pxContext.h"
#include "pxUtil.h"
#include <algorithm>
#include <ctime>
#include <cstdlib>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
#include <GLES2/gl2.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#ifdef WIN32
#include <GL/wglew.h>
#endif // WIN32
#ifdef PX_PLATFORM_GLUT
#include <GL/glut.h>
#endif
#include <GL/gl.h>
#endif //PX_PLATFORM_WAYLAND_EGL
#endif

#include "pxSharedContext.h"
#include "pxTimer.h"

#define PX_TEXTURE_MIN_FILTER GL_LINEAR
#define PX_TEXTURE_MAG_FILTER GL_LINEAR

#define CONTEXT_GC_THROTTLE_SECS_DEFAULT 5

// Values must match pxCanvas.h // TODO FIX
#define  CANVAS_W   1280
#define  CANVAS_H    720

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

pxContextSurfaceNativeDesc  defaultContextSurface;
pxContextSurfaceNativeDesc* currentContextSurface = &defaultContextSurface;

pxContextFramebufferRef defaultFramebuffer(new pxContextFramebuffer());
pxContextFramebufferRef currentFramebuffer = defaultFramebuffer;


#ifdef RUNINMAIN
extern rtScript script;
#else
extern uv_async_t gcTrigger;
#endif
extern pxContext context;
rtThreadQueue* gUIThreadQueue = new rtThreadQueue();
double lastContextGarbageCollectTime = 0;
double garbageCollectThrottleInSeconds = CONTEXT_GC_THROTTLE_SECS_DEFAULT;

int currentGLProgram = -1;

#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
extern EGLContext defaultEglContext;
extern EGLDisplay defaultEglDisplay;
extern EGLSurface defaultEglSurface;
#endif //PX_PLATFORM_GENERIC_EGL || PX_PLATFORM_WAYLAND_EGL

struct filterXYR
{
    float x; float y; float r;
};

// TODO get rid of this global crap

static int gResW, gResH;
static pxMatrix4f gMatrix;
static float gAlpha = 1.0;
uint32_t gRenderTick = 0;
rtMutex gRenderTickMutex;
std::vector<pxTexture*> textureList;
rtMutex textureListMutex;
#ifdef ENABLE_BACKGROUND_TEXTURE_CREATION
rtMutex contextLock;
#endif //ENABLE_BACKGROUND_TEXTURE_CREATION
bool gFlipRendering = false;


pxError lockContext()
{
#ifdef ENABLE_BACKGROUND_TEXTURE_CREATION
  contextLock.lock();
#endif //ENABLE_BACKGROUND_TEXTURE_CREATION
  return PX_OK;
}

pxError unlockContext()
{
#ifdef ENABLE_BACKGROUND_TEXTURE_CREATION
  contextLock.unlock();
#endif //ENABLE_BACKGROUND_TEXTURE_CREATION
  return PX_OK;
}

pxError addToTextureList(pxTexture* texture)
{
  textureListMutex.lock();
  textureList.push_back(texture);
  textureListMutex.unlock();
  return PX_OK;
}

pxError removeFromTextureList(pxTexture* texture)
{
  textureListMutex.lock();
  for(std::vector<pxTexture*>::iterator it = textureList.begin(); it != textureList.end(); ++it)
  {
    if ((*it) == texture)
    {
      textureList.erase(it);
      break;
    }
  }
  textureListMutex.unlock();
  return PX_OK;
}

pxError ejectNotRecentlyUsedTextureMemory(int64_t bytesNeeded, int64_t targetMemoryAmount,
                                          bool clearAllOffscreen, uint32_t maxAge=5)
{
  //rtLogDebug("attempting to eject %" PRId64 " bytes of texture memory with max age %u", bytesNeeded, maxAge);
#if !defined(DISABLE_TEXTURE_EJECTION)
  int numberEjected = 0;
  int64_t beforeTextureMemoryUsage = 0;
  lockContext();
  beforeTextureMemoryUsage = context.currentTextureMemoryUsageInBytes();
  unlockContext();

  uint32_t currentRenderTick = 0;
  {
    rtMutexLockGuard renderTickMutexGuard(gRenderTickMutex);
    currentRenderTick = gRenderTick;
  }

  textureListMutex.lock();
  for(std::vector<pxTexture*>::iterator it = textureList.begin(); it != textureList.end(); ++it)
  {
    pxTexture* texture = (*it);
    uint32_t lastRenderTickAge = currentRenderTick - texture->lastRenderTick();
    bool textureIsSetupForRendering = texture->setupForRendering();
    if (lastRenderTickAge > maxAge && textureIsSetupForRendering)
    {
      numberEjected++;
      texture->unloadTextureData();
      int64_t currentTextureMemory = 0;
      lockContext();
      currentTextureMemory = context.currentTextureMemoryUsageInBytes();
      unlockContext();
      if (!clearAllOffscreen && (currentTextureMemory <= targetMemoryAmount) &&
          (beforeTextureMemoryUsage - currentTextureMemory) > bytesNeeded)
      {
        break;
      }
    }
  }
  textureListMutex.unlock();

  if (numberEjected > 0)
  {
    int64_t afterTextureMemoryUsage = 0;
    lockContext();
    afterTextureMemoryUsage = context.currentTextureMemoryUsageInBytes();
    unlockContext();
    rtLogWarn("%d textures have been ejected and %" PRId64 " bytes of texture memory has been freed",
        numberEjected, (beforeTextureMemoryUsage - afterTextureMemoryUsage));
  }
#else
  (void)bytesNeeded;
  (void)targetMemoryAmount;
  (void)clearAllOffscreen;
  (void)maxAge;
#endif //!DISABLE_TEXTURE_EJECTION
  return PX_OK;
}

// assume premultiplied
static const char *fSolidShaderText =
  "#ifdef GL_ES \n"
  "  precision mediump float; \n"
  "#endif \n"
  "uniform float u_alpha;"
  "uniform vec4 a_color;"
  "void main()"
  "{"
  "  gl_FragColor = a_color*u_alpha;"
  "}";

// assume premultiplied
static const char *fTextureShaderText =
  "#ifdef GL_ES \n"
  "  precision mediump float; \n"
  "#endif \n"
  "uniform sampler2D s_texture;"
  "uniform float u_alpha;"
  "varying vec2 v_uv;"
  "void main()"
  "{"
  "  gl_FragColor = texture2D(s_texture, v_uv) * u_alpha;"
  "}";

// assume premultiplied
static const char *fTextureBorderShaderText =
  "#ifdef GL_ES \n"
  "  precision mediump float; \n"
  "#endif \n"
  "uniform sampler2D s_texture;"
  "uniform float u_alpha;"
  "uniform vec4 u_color;"
  "varying vec2 v_uv;"
  "void main()"
  "{"
  "  gl_FragColor = texture2D(s_texture, vec2(v_uv.s, 1.0 - v_uv.t)) * u_alpha * u_color;"
  "}";

// assume premultiplied
static const char *fTextureMaskedShaderText =
  "#ifdef GL_ES \n"
  "  precision mediump float; \n"
  "#endif \n"
  "uniform sampler2D s_texture;"
  "uniform sampler2D s_mask;"
  "uniform float u_alpha;"
  "uniform float u_doInverted;"
  "varying vec2 v_uv;"
  "void main()"
  "{"
  "float tex_a =  texture2D(s_mask, v_uv).a;"
  "float     a = (1.0 - u_doInverted) * (u_alpha * (      tex_a)) +" // do NORMAL   mask
  "              (      u_doInverted) * (u_alpha * (1.0 - tex_a)) ;" // do INVERTED mask
  "  gl_FragColor = texture2D(s_texture, v_uv) * a;"
  "}";

// assume premultiplied
static const char *fATextureShaderText =
  "#ifdef GL_ES \n"
  "  precision mediump float; \n"
  "#endif \n"
  "uniform sampler2D s_texture;"
  "uniform float u_alpha;"
  "uniform vec4 a_color;"
  "varying vec2 v_uv;"
  "void main()"
  "{"
  "  float a = u_alpha * texture2D(s_texture, v_uv).a;"
  "  gl_FragColor = a_color*a;"
  "}";

/*static*/ const char *vShaderText =
  "uniform vec2 u_resolution;"
  "uniform mat4 amymatrix;"
  "uniform bool u_flipY;"
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
  "  int yScale = -1;"
  "  if (u_flipY) { yScale = 1; }"
  "  gl_Position =  clipSpace * vec4(1, yScale, 1, 1);"
  "  v_uv = uv;"
  "}";

// assume premultiplied
static const char *fLinearBlurShaderText =

  "#ifdef GL_ES\n"
  "  precision mediump float;\n"
  "#endif\n"

  "uniform float     u_alpha;"
  "uniform vec4      a_color;"

  "varying vec2      v_uv;"

  "uniform vec2      u_resolution;"
  "uniform int       u_kernelRadius;"

  "uniform vec2      u_direction;"
  "uniform vec4      u_shadowColor;"

  "uniform sampler2D s_texture;"

  "vec4 blur5(sampler2D image, vec2 uv, vec2 resolution, vec2 direction)"
  "{"
  "  vec4 color = vec4(0.0);"
  "  vec2 off1  = vec2(1.3333333333333333) * direction;"
  "  color += texture2D(image, uv) * 0.29411764705882354;"
  "  color += texture2D(image, uv + (off1 / resolution)) * 0.35294117647058826;"
  "  color += texture2D(image, uv - (off1 / resolution)) * 0.35294117647058826;"
  "  return color;"
  "}"

  "vec4 blur9(sampler2D image, vec2 uv, vec2 resolution, vec2 direction)"
  "{"
  "  vec4 color = vec4(0.0);"
  "  vec2 off1 = vec2(1.3846153846) * direction;"
  "  vec2 off2 = vec2(3.2307692308) * direction;"
  "  color += texture2D(image, uv) * 0.2270270270;"
  "  color += texture2D(image, uv + (off1 / resolution)) * 0.3162162162;"
  "  color += texture2D(image, uv - (off1 / resolution)) * 0.3162162162;"
  "  color += texture2D(image, uv + (off2 / resolution)) * 0.0702702703;"
  "  color += texture2D(image, uv - (off2 / resolution)) * 0.0702702703;"
  "  return color;"
  "}"

  "vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction)"
  "{"
  "  vec4 color = vec4(0.0);"
  "  vec2 off1 = vec2(1.4117647058823530) * direction;"
  "  vec2 off2 = vec2(3.2941176470588234) * direction;"
  "  vec2 off3 = vec2(5.1764705882352940) * direction;"
  "  color += texture2D(image, uv) * 0.1964825501511404;"
  "  color += texture2D(image, uv + (off1 / resolution)) * 0.2969069646728344;"
  "  color += texture2D(image, uv - (off1 / resolution)) * 0.2969069646728344;"
  "  color += texture2D(image, uv + (off2 / resolution)) * 0.09447039785044732;"
  "  color += texture2D(image, uv - (off2 / resolution)) * 0.09447039785044732;"
  "  color += texture2D(image, uv + (off3 / resolution)) * 0.010381362401148057;"
  "  color += texture2D(image, uv - (off3 / resolution)) * 0.010381362401148057;"
  "  return color;"
  "}"

  "void main()"
  "{"
  "\n #if 1 \n"
  "  if (u_kernelRadius == 1)"
  "  {"
"    gl_FragColor = blur5(s_texture, v_uv, u_resolution, u_direction);"
//  "   gl_FragColor =  texture2D(s_texture, v_uv) + vec4(1.0, 0.0, 0.0,  1.0);" // DEBUG - RED
  "  }"
  "  else if (u_kernelRadius == 2)"
  "  {"
"      gl_FragColor = blur9(s_texture, v_uv, u_resolution, u_direction);"
//  "   gl_FragColor =  texture2D(s_texture, v_uv) + vec4(0.0, 1.0, 0.0,  1.0);" // DEBUG - GREEN
  "  }"
  "  else"
  "  {"
  "    gl_FragColor= blur13(s_texture, v_uv, u_resolution, u_direction);"
//  "   gl_FragColor =  texture2D(s_texture, v_uv) + vec4(0.0, 0.0, 1.0,  1.0);" // DEBUG - BLUE
  "  }"
  ""
  "\n #else\n"
  ""
  "  gl_FragColor = texture2D(s_texture, v_uv)" // just copy
  ""
  "\n#endif\n"
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
  pxFBOTexture(bool antiAliasing, bool alphaOnly, bool depthBuffer) : mWidth(0), mHeight(0), mFramebufferId(0), mTextureId(0), mBindTexture(true), mAlphaOnly(alphaOnly), mDepthBuffer(depthBuffer)

#if (defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)) && !defined(PXSCENE_DISABLE_PXCONTEXT_EXT)
        ,mAntiAliasing(antiAliasing)
#endif
, mDepthrenderbuffer(0)
  {
    UNUSED_PARAM(antiAliasing);

#ifndef SPARK_ALPHA_FBO_SUPPORT
    //disable alpha fbo support if the feature is disabled
    mAlphaOnly = false;
#endif //SPARK_ALPHA_FBO_SUPPORT

    mTextureType = PX_TEXTURE_FRAME_BUFFER;
  }

  ~pxFBOTexture() { deleteTexture(); }

  void createFboTexture(int w, int h)
  {
    if (mFramebufferId != 0 && mTextureId != 0)
    {
      deleteTexture();
    }

    mWidth  = w;
    mHeight = h;
    int32_t bytesPerPixel = mAlphaOnly ? 1 : 4;
    if (!context.isTextureSpaceAvailable(this, true, bytesPerPixel))
    {
      rtLogDebug("Not enough texture memory to create FBO");
      return;
    }

    glGenFramebuffers(1, &mFramebufferId);
    glGenTextures(1, &mTextureId);

    glBindTexture(GL_TEXTURE_2D, mTextureId); TRACK_TEX_CALLS();
    if (mAlphaOnly)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                 mWidth, mHeight, 0, GL_ALPHA,
                 GL_UNSIGNED_BYTE, NULL);
    }
    else
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 mWidth, mHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (mAlphaOnly)
    {
      context.adjustCurrentTextureMemorySize(mWidth*mHeight);
    }
    else
    {
      context.adjustCurrentTextureMemorySize(mWidth*mHeight*4);
    }
    mBindTexture = true;
  }

  pxError resizeTexture(int w, int h)
  {
    if (mWidth != w || mHeight != h ||
        mFramebufferId == 0 || mTextureId == 0)
    {
      createFboTexture(w, h);
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
#if (defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)) && !defined(PXSCENE_DISABLE_PXCONTEXT_EXT)
      if (mAntiAliasing)
      {
        GLint currentFBO = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);
        GLenum discardAttachments[] = { GL_DEPTH_ATTACHMENT };
        glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discardAttachments);
        glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
      }
#endif

      glDeleteFramebuffers(1, &mFramebufferId);
      mFramebufferId = 0;
    }

    if (mTextureId != 0)
    {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
      if (mAlphaOnly)
      {
        context.adjustCurrentTextureMemorySize(-1*mWidth*mHeight);
      }
      else
      {
        context.adjustCurrentTextureMemorySize(-1*mWidth*mHeight*4);
      }
    }

    if (mDepthrenderbuffer != 0)
    {
      glDeleteRenderbuffers(1, &mDepthrenderbuffer);
      mDepthrenderbuffer = 0;
    }

    return PX_OK;
  }

  virtual unsigned int getNativeId()
  {
    return mTextureId;
  }

  virtual pxError prepareForRendering()
  {
    if (mFramebufferId == 0 || mTextureId == 0)
    {
      return PX_FAIL;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);   TRACK_FBO_CALLS();
    if (mBindTexture)
    {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_2D, mTextureId, 0);

#if (defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)) && !defined(PXSCENE_DISABLE_PXCONTEXT_EXT)
      if (mAntiAliasing)
      {
        glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0, 2);
      }
#endif

      if (mDepthBuffer)
      {
        if (mDepthrenderbuffer == 0)
          glGenRenderbuffers(1, &mDepthrenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthrenderbuffer);
#if (defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL))
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, mWidth, mHeight);
#else
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mWidth, mHeight);
#endif //(defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL))
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthrenderbuffer);
      }

      GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);

      if (result != GL_FRAMEBUFFER_COMPLETE)
      {
        if ((mWidth != 0) && (mHeight != 0))
        {
          if (mAlphaOnly)
          {
            rtLogDebug("unable to create fbo that is alpha only.  trying to create a standard fbo");
            deleteTexture();
            mAlphaOnly = false;
            //recreate the FBO with non-alpha only for platforms that don't support alpha only backings
            createFboTexture(mWidth, mHeight);
            return prepareForRendering();
          }
          rtLogWarn("error setting the render surface: %d", result);
          return PX_FAIL;
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

  virtual pxError getOffscreen(pxOffscreen& o)
  {
    o.init(mWidth,mHeight);
    glReadPixels(0,0,mWidth,mHeight,GL_RGBA,GL_UNSIGNED_BYTE,(void*)o.base());
    o.setUpsideDown(true);

    return PX_OK;
  }

  virtual int width() { return mWidth; }
  virtual int height() { return mHeight; }

private:
  int mWidth;
  int mHeight;
  GLuint mFramebufferId;
  GLuint mTextureId;
  bool mBindTexture;
  bool mAlphaOnly;
  bool mDepthBuffer;

#if (defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)) && !defined(PXSCENE_DISABLE_PXCONTEXT_EXT)
  bool mAntiAliasing;
#endif
  GLuint mDepthrenderbuffer;

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
typedef rtRef<pxTextureOffscreen> pxTextureOffscreenRef;

struct DecodeImageData
{
  DecodeImageData(pxTextureOffscreenRef t) : textureOffscreen(t)
  {
  }
  pxTextureOffscreenRef textureOffscreen;

};

void onOffscreenCleanupComplete(void* context, void*);
void cleanupOffscreen(void* data);

class pxTextureOffscreen : public pxTexture
{
public:
  pxTextureOffscreen() : mOffscreen(), mInitialized(false), mTextureName(0),
                         mTextureUploaded(false), mWidth(0), mHeight(0), mOffscreenMutex(),
                         mFreeOffscreenDataRequested(false),
                         mMipmapCreated(false), mTextureListener(NULL), mTextureListenerMutex(),
                         mReadyForRendering(false), mRenderingMutex(), mSetupForRendering(false)
  {
    mTextureType = PX_TEXTURE_OFFSCREEN;
    {
      rtMutexLockGuard renderTickMutexGuard(gRenderTickMutex);
      mLastRenderTick = gRenderTick;
    }
    addToTextureList(this);
  }

  pxTextureOffscreen(pxOffscreen& o)
                                     : mOffscreen(), mInitialized(false), mTextureName(0),
                                       mTextureUploaded(false), mWidth(0), mHeight(0), mOffscreenMutex(),
                                       mFreeOffscreenDataRequested(false),
                                       mMipmapCreated(false), mTextureListener(NULL), mTextureListenerMutex(),
                                       mReadyForRendering(false), mRenderingMutex(), mSetupForRendering(false)
  {
    mTextureType = PX_TEXTURE_OFFSCREEN;
    createTexture(o);
    {
      rtMutexLockGuard renderTickMutexGuard(gRenderTickMutex);
      mLastRenderTick = gRenderTick;
    }
    addToTextureList(this);
  }

  ~pxTextureOffscreen() { deleteTexture(); removeFromTextureList(this);};

  virtual unsigned int getNativeId()
  {
    return mTextureName;
  }

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

    mFreeOffscreenDataRequested = false;
    mOffscreenMutex.unlock();

    mInitialized = true;

    mTextureListenerMutex.lock();
    mRenderingMutex.lock();
    mReadyForRendering = true;
    mRenderingMutex.unlock();
    if (mTextureListener != NULL)
    {
      mTextureListener->textureReady();
    }
    mTextureListenerMutex.unlock();

    return PX_OK;
  }

  virtual void setUpsideDown(bool upsideDown)
  {
    // TODO - revisit
    pxOffscreen tempOffscreen;
    tempOffscreen.init(mOffscreen.width(), mOffscreen.height());
    tempOffscreen.setUpsideDown(upsideDown);
    mOffscreen.blit(tempOffscreen);
    mOffscreen = tempOffscreen;
  }

  virtual pxError prepareForRendering()
  {
    if (mTextureName == 0 && context.isTextureSpaceAvailable(this, false))
    {
      glActiveTexture(GL_TEXTURE1);
      glGenTextures(1, &mTextureName);
      glBindTexture(GL_TEXTURE_2D, mTextureName);   TRACK_TEX_CALLS();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                   mOffscreen.width(), mOffscreen.height(), 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, mOffscreen.base());
      if (mDownscaleSmooth && !mMipmapCreated)
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        mMipmapCreated = true;
      }
      context.adjustCurrentTextureMemorySize(mOffscreen.width()*mOffscreen.height()*4, false);
#ifdef ENABLE_BACKGROUND_TEXTURE_CREATION
      mOffscreen.term();
#endif //ENABLE_BACKGROUND_TEXTURE_CREATION
    }
    return PX_OK;
  }

  virtual bool initialized()
  {
    return (mTextureName != 0);
  }

  virtual bool readyForRendering()
  {
    bool result = false;
    mRenderingMutex.lock();
    result = mReadyForRendering;
    mRenderingMutex.unlock();
    return result;
  }

  virtual bool setupForRendering()
  {
    bool result = false;
    mRenderingMutex.lock();
    result = mSetupForRendering;
    mRenderingMutex.unlock();
    return result;
  }

  virtual pxError deleteTexture()
  {
    rtLogDebug("pxTextureOffscreen::deleteTexture()");

    unloadTextureData();

    mInitialized = false;
    mRenderingMutex.lock();
    mReadyForRendering = false;
    mRenderingMutex.unlock();
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
      mRenderingMutex.lock();
      mReadyForRendering = false;
      mSetupForRendering = false;
      mRenderingMutex.unlock();
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

  virtual pxError setTextureListener(pxTextureListener* textureListener)
  {
    mTextureListenerMutex.lock();
    mTextureListener = textureListener;
    mTextureListenerMutex.unlock();
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
      if (mTextureName == 0 && !context.isTextureSpaceAvailable(this))
      {
        //attempt to free texture memory
        int64_t textureMemoryNeeded = context.textureMemoryOverflow(this);
        context.ejectTextureMemory(textureMemoryNeeded);
        if (!context.isTextureSpaceAvailable(this))
        {
          rtLogError("not enough texture memory remaining to create texture");
          mInitialized = false;
          mRenderingMutex.lock();
          mReadyForRendering = false;
          mRenderingMutex.unlock();
          freeOffscreenDataInBackground();
          return PX_FAIL;
        }
        else if (!mInitialized)
        {
          return PX_NOTINITIALIZED;
        }
      }
      if (mTextureName == 0)
      {
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
        if (mDownscaleSmooth)
        {
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
          glGenerateMipmap(GL_TEXTURE_2D);
          mMipmapCreated = true;
        }
        context.adjustCurrentTextureMemorySize(mOffscreen.width()*mOffscreen.height()*4);
      }
      else
      {
        glBindTexture(GL_TEXTURE_2D, mTextureName);   TRACK_TEX_CALLS();
        if (mDownscaleSmooth && !mMipmapCreated)
        {
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
          glGenerateMipmap(GL_TEXTURE_2D);
          mMipmapCreated = true;
        }
      }
      mTextureUploaded = true;
      //free up unneeded offscreen memory
      freeOffscreenDataInBackground();
      mRenderingMutex.lock();
      mSetupForRendering = true;
      mRenderingMutex.unlock();
    }
    else
    {
      glBindTexture(GL_TEXTURE_2D, mTextureName);   TRACK_TEX_CALLS();
      if (mDownscaleSmooth && !mMipmapCreated)
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        mMipmapCreated = true;
      }
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
        //attempt to free texture memory
        int64_t textureMemoryNeeded = context.textureMemoryOverflow(this);
        context.ejectTextureMemory(textureMemoryNeeded);
        if (!context.isTextureSpaceAvailable(this))
        {
          rtLogError("not enough texture memory remaining to create texture");
          mInitialized = false;
          mRenderingMutex.lock();
          mReadyForRendering = false;
          mRenderingMutex.unlock();
          freeOffscreenDataInBackground();
          return PX_FAIL;
        }
        else if (!mInitialized)
        {
          return PX_NOTINITIALIZED;
        }
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
      mRenderingMutex.lock();
      mSetupForRendering = true;
      mRenderingMutex.unlock();
    }
    else
    {
      glBindTexture(GL_TEXTURE_2D, mTextureName);   TRACK_TEX_CALLS();
    }

    glUniform1i(mLoc, 2);

    return PX_OK;
  }

  virtual pxError getOffscreen(pxOffscreen& /*o*/)
  {
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
    DecodeImageData *imageData = new DecodeImageData(this);
    rtThreadTask *task = new rtThreadTask(cleanupOffscreen, imageData, "");
    mainThreadPool->executeTask(task);
  }

  pxOffscreen mOffscreen;

  bool mInitialized;
  GLuint mTextureName;
  bool mTextureUploaded;
  int mWidth;
  int mHeight;
  rtMutex mOffscreenMutex;
  bool mFreeOffscreenDataRequested;
  bool mMipmapCreated;
  pxTextureListener* mTextureListener;
  rtMutex mTextureListenerMutex;
  bool mReadyForRendering;
  rtMutex mRenderingMutex;
  bool mSetupForRendering;

}; // CLASS - pxTextureOffscreen

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
      if (gUIThreadQueue)
      {
        gUIThreadQueue->addTask(onOffscreenCleanupComplete, data, NULL);
      }
    }
    else
    {
      if (gUIThreadQueue)
      {
        gUIThreadQueue->addTask(onOffscreenCleanupComplete, data, NULL);
      }
    }
  }
}

//====================================================================================================================================================================================

class pxTextureAlpha : public pxTexture
{
public:
  pxTextureAlpha() : mDrawWidth(0.0), mDrawHeight (0.0), mImageWidth(0.0),
                     mImageHeight(0.0), mTextureId(0), mInitialized(false),
                     mBuffer(NULL)
  {
    mTextureType = PX_TEXTURE_ALPHA;
  }

  pxTextureAlpha(float w, float h, float iw, float ih, void* buffer)
    : mDrawWidth(w),    mDrawHeight (h),
      mImageWidth(iw), mImageHeight(ih),
      mTextureId(0), mInitialized(false), mBuffer(NULL)
  {
    mTextureType = PX_TEXTURE_ALPHA;


    // TODO consider iw,ih as ints rather than floats...
    int32_t bw = (int32_t)iw;
    int32_t bh = (int32_t)ih;

    if (buffer)
    {
      // copy the pixels
      int bitmapSize = static_cast<int>(ih*iw);
      mBuffer = malloc(bitmapSize);
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
    }
    else
    {
      int bitmapSize = static_cast<int>(ih*iw);
      mBuffer = calloc(bitmapSize, sizeof(char));
    }

// TODO Moved this to bindTexture because of more pain from JS thread calls
//    createTexture(w, h, iw, ih);
  }

  ~pxTextureAlpha()
  {
    if(mBuffer)
    {
      free(mBuffer);
      mBuffer  = 0;
    }
    deleteTexture();
  }

  void createAlphaTexture(float w, float h, float iw, float ih)
  {
    if (mTextureId != 0)
    {
      deleteTexture();
    }

    if(iw == 0 || ih == 0)
    {
      rtLogError("pxTextureAlpha::createAlphaTexture() - DIMENSIONLESS ");
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
      static_cast<GLsizei>(iw),
      static_cast<GLsizei>(ih),
      0,
      GL_ALPHA,
      GL_UNSIGNED_BYTE,
      mBuffer
    );
    context.adjustCurrentTextureMemorySize(static_cast<GLsizei>(iw*ih));

    mInitialized = true;
  }

  virtual pxError updateTexture(int x, int y, int w, int h,  void* buffer)
  {
    // TODO Moved to here because of js threading issues
    if (!mInitialized) createAlphaTexture(mDrawWidth,mDrawHeight,mImageWidth,mImageHeight);
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }

    glBindTexture(GL_TEXTURE_2D, mTextureId);   TRACK_TEX_CALLS();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D,
        0,x,y,w,h,GL_ALPHA,GL_UNSIGNED_BYTE,buffer);

    return PX_OK;
  }

  virtual pxError deleteTexture()
  {
    if (mTextureId != 0)
    {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
      context.adjustCurrentTextureMemorySize(static_cast<int64_t>(-1*mImageWidth*mImageHeight));
    }
    mInitialized = false;
    return PX_OK;
  }

  virtual pxError bindGLTexture(int tLoc)
  {
    // TODO Moved to here because of js threading issues
    if (!mInitialized) createAlphaTexture(mDrawWidth,mDrawHeight,mImageWidth,mImageHeight);
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

  virtual int width()  {return static_cast<int>(mDrawWidth);  }
  virtual int height() {return static_cast<int>(mDrawHeight); }

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

class solidShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    mPosLoc = 0;
    mUVLoc = 1;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc,  "uv");
  }

  virtual void postlink()
  {
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc     = getUniformLocation("amymatrix");
    mFlipYLoc      = getUniformLocation("u_flipY");
    mColorLoc      = getUniformLocation("a_color");
    mAlphaLoc      = getUniformLocation("u_alpha");
  }

public:
   pxError draw(int resW, int resH, float* matrix, float alpha,
            GLenum mode,
            const void* pos,
            int count,
            const float* color)
  {
    use(); // change program... if needed

    glUniform2f(mResolutionLoc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1i(mFlipYLoc, gFlipRendering);
    glUniform1f(mAlphaLoc, alpha);
    glUniform4fv(mColorLoc, 1, color);

    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glEnableVertexAttribArray(mPosLoc);
    glDrawArrays(mode, 0, count);  TRACK_DRAW_CALLS();
    glDisableVertexAttribArray(mPosLoc);

    return PX_OK;
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;
  GLint mFlipYLoc;

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
    glBindAttribLocation(mProgram, mUVLoc,  "uv");
  }

  virtual void postlink()
  {
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc     = getUniformLocation("amymatrix");
    mFlipYLoc      = getUniformLocation("u_flipY");
    mColorLoc      = getUniformLocation("a_color");
    mAlphaLoc      = getUniformLocation("u_alpha");
    mTextureLoc    = getUniformLocation("s_texture");
  }

public:
  pxError draw(int resW, int resH, float* matrix, float alpha,
            GLenum mode,
            int count,
            const void* pos,
            const void* uv,
            pxTextureRef texture,
            const float* color)
  {
    use(); // change program... if needed

    glUniform2f(mResolutionLoc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1f(mAlphaLoc, alpha);
    glUniform1i(mFlipYLoc, gFlipRendering);
    glUniform4fv(mColorLoc, 1, color);

    if (texture->bindGLTexture(mTextureLoc) != PX_OK)
    {
      return PX_FAIL;
    }

    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glVertexAttribPointer(mUVLoc, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(mPosLoc);
    glEnableVertexAttribArray(mUVLoc);
    glDrawArrays(mode, 0, count);  TRACK_DRAW_CALLS();
    glDisableVertexAttribArray(mPosLoc);
    glDisableVertexAttribArray(mUVLoc);

    return PX_OK;
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;
  GLint mFlipYLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mColorLoc;
  GLint mAlphaLoc;

  GLint mTextureLoc;

}; //CLASS - aTextureShaderProgram

aTextureShaderProgram *gATextureShader = NULL;


//====================================================================================================================================================================================

class aLinearBlurShaderProgram: public shaderProgram
{
  public:
  aLinearBlurShaderProgram()
  {
    mResolutionLoc    = -1;
    mMatrixLoc        = -1;
    mKernelRadiusLoc  = -1;
    mBlurDirectionLoc = -1;
    mShadowColorLoc   = -1;
    mPosLoc           = -1;
    mUVLoc            = -1;
    mColorLoc         = -1;
    mAlphaLoc         = -1;
    mTextureLoc       = -1;
  }

  protected:
  virtual void prelink()
  {
    mPosLoc = 0;
    mUVLoc  = 1;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc,  "uv");
  }

  virtual void postlink()
  {
    mResolutionLoc    = getUniformLocation("u_resolution");
    mMatrixLoc        = getUniformLocation("amymatrix");
    mColorLoc         = getUniformLocation("a_color");
    mAlphaLoc         = getUniformLocation("u_alpha");
    mTextureLoc       = getUniformLocation("s_texture");

    mKernelRadiusLoc  = getUniformLocation("u_kernelRadius");
    mBlurDirectionLoc = getUniformLocation("u_direction");

    mShadowColorLoc   = getUniformLocation("u_shadowColor");
  }

public:

     pxError draw(int resW, int resH, float* matrix, float alpha,
               pxTextureRef texture,
               GLenum mode,
               const void* pos,
               const void* uv,
               int count, void *options)
  {
      pxTextEffects* pe;
      if (options)
      {
          pe = static_cast<pxTextEffects*>(options);
      } else {
          return PX_FAIL;  // could no draw w/o shadow/hightlihgt options
      }

    if(mResolutionLoc == -1)
    {
      return PX_FAIL;  // not ready
    }

    use(); // change program... if needed

    glUniform2f(mResolutionLoc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);

    glUniform1f(mAlphaLoc, alpha);
    glUniform4fv(mShadowColorLoc, 1, pe->shadowColor);
    glUniform1i(mKernelRadiusLoc, (int) pe->shadowRadius);

    float dir[] = { pe->shadowX, pe->shadowY };
    glUniform2fv(mBlurDirectionLoc, 1, dir);

    if (texture->bindGLTexture(mTextureLoc) != PX_OK)
    {
      return PX_FAIL;
    }

    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glVertexAttribPointer(mUVLoc,  2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(mPosLoc);
    glEnableVertexAttribArray(mUVLoc);
    glDrawArrays(mode, 0, count);  TRACK_DRAW_CALLS();
    glDisableVertexAttribArray(mPosLoc);
    glDisableVertexAttribArray(mUVLoc);

    return PX_OK;
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;

  GLint mKernelRadiusLoc;
  GLint mBlurDirectionLoc;
  GLint mShadowColorLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mColorLoc;
  GLint mAlphaLoc;

  GLint mTextureLoc;

}; //CLASS - aTextureBlurShaderProgram

aLinearBlurShaderProgram *gATextureBlurShader = NULL;

//====================================================================================================================================================================================

class textureShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    mPosLoc = 0;
    mUVLoc = 1;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc,  "uv");
  }

  virtual void postlink()
  {
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc     = getUniformLocation("amymatrix");
    mFlipYLoc      = getUniformLocation("u_flipY");
    mAlphaLoc      = getUniformLocation("u_alpha");
    mTextureLoc    = getUniformLocation("s_texture");
  }

public:
  pxError draw(int resW, int resH, float* matrix, float alpha,
            int count,
            const void* pos, const void* uv,
            pxTextureRef texture,
            int32_t stretchX, int32_t stretchY)
  {
    use(); // change program... if needed

    glUniform2f(mResolutionLoc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1i(mFlipYLoc, gFlipRendering);
    glUniform1f(mAlphaLoc, alpha);

    if (texture->bindGLTexture(mTextureLoc) != PX_OK)
    {
      return PX_FAIL;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		    (stretchX==pxContextStretch::REPEAT)?GL_REPEAT:GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		    (stretchY==pxContextStretch::REPEAT)?GL_REPEAT:GL_CLAMP_TO_EDGE);

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
  GLint mFlipYLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mAlphaLoc;

  GLint mTextureLoc;

}; //CLASS - textureShaderProgram

textureShaderProgram *gTextureShader = NULL;

class textureBorderShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    mPosLoc = 0;
    mUVLoc = 1;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc,  "uv");
  }

  virtual void postlink()
  {
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc     = getUniformLocation("amymatrix");
    mFlipYLoc      = getUniformLocation("u_flipY");
    mAlphaLoc      = getUniformLocation("u_alpha");
    mColorLoc      = getUniformLocation("u_color");
    mTextureLoc    = getUniformLocation("s_texture");
  }

public:
  pxError draw(int resW, int resH, float* matrix, float alpha,
               int count,
               const void* pos, const void* uv,
               pxTextureRef texture,
               int32_t stretchX, int32_t stretchY, const float* color = NULL)
  {
    use(); // change program... if needed

    glUniform2f(mResolutionLoc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1i(mFlipYLoc, gFlipRendering);
    glUniform1f(mAlphaLoc, alpha);
    if (color != NULL)
    {
      glUniform4fv(mColorLoc, 1, color);
    }
    else
    {
      static float defaultColor[4] = {1.0, 1.0, 1.0, 1.0};
      glUniform4fv(mColorLoc, 1, defaultColor);
    }

    if (texture->bindGLTexture(mTextureLoc) != PX_OK)
    {
      return PX_FAIL;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    (stretchX==pxContextStretch::REPEAT)?GL_REPEAT:GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    (stretchY==pxContextStretch::REPEAT)?GL_REPEAT:GL_CLAMP_TO_EDGE);

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
  GLint mFlipYLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mAlphaLoc;
  GLint mColorLoc;

  GLint mTextureLoc;

}; //CLASS - textureBorderShaderProgram

textureBorderShaderProgram *gTextureBorderShader = NULL;

//====================================================================================================================================================================================

class textureMaskedShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    mPosLoc = 0;
    mUVLoc = 1;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc,  "uv");
  }

  virtual void postlink()
  {
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc     = getUniformLocation("amymatrix");
    mFlipYLoc      = getUniformLocation("u_flipY");
    mAlphaLoc      = getUniformLocation("u_alpha");
    mInvertedLoc   = getUniformLocation("u_doInverted");
    mTextureLoc    = getUniformLocation("s_texture");
    mMaskLoc       = getUniformLocation("s_mask");
  }

public:
  pxError draw(int resW, int resH, float* matrix, float alpha,
            int count,
            const void* pos,
            const void* uv,
            pxTextureRef texture,
            pxTextureRef mask,
            pxContextMaskOperation maskOp = pxContextMaskOperation::NORMAL)
  {
    use(); // change program... if needed

    glUniform2f(mResolutionLoc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1i(mFlipYLoc, gFlipRendering);
    glUniform1f(mAlphaLoc, alpha);
    glUniform1f(mInvertedLoc, static_cast<GLfloat>((maskOp == pxContextMaskOperation::NORMAL) ? 0.0 : 1.0));


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
  GLint mFlipYLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mAlphaLoc;
  GLint mInvertedLoc;

  GLint mTextureLoc;
  GLint mMaskLoc;

}; //CLASS - textureMaskedShaderProgram

textureMaskedShaderProgram *gTextureMaskedShader = NULL;

//====================================================================================================================================================================================
pxContextFramebufferRef applyBlurSettings(pxContextFramebufferRef src, const pxTextEffects* pe, filterXYR *filters, size_t count)
{
    if(filters == NULL)
    {
        rtLogError("No filters to apply.");
        return NULL;
    }
    int w = src->width();
    int h = src->height();

    //
    // FBO: 'output1'
    //
    pxContextFramebufferRef output1 = context.createFramebuffer(w,h);
    context.setFramebuffer(output1);
    context.clear(w,h);

    //
    // FBO: 'output0'
    //
    pxContextFramebufferRef output0 = context.createFramebuffer(w,h);
    context.setFramebuffer(output0);
    context.clear(w,h);

    pxContextFramebufferRef fbo_src = src;      // initial draw of SRC to OUTPUT0
    pxContextFramebufferRef fbo_dst = output0;

    pxTextEffects options;
    memcpy(&options, pe, sizeof(options));

    size_t i = 0;
    do
    {
        // Render pass with Blur EFFECT shader...
        options.shadowX      = filters[i].x;
        options.shadowY      = filters[i].y;
        options.shadowRadius = filters[i].r;

        context.setFramebuffer(fbo_dst);
        context.drawEffect(0, 0, w, h, fbo_src->getTexture(), gATextureBlurShader, &options);

        bool toggle = (i++ % 2); // TOGGLE !! ... (0 % 2) = false ... at start

        // Alternate between Source Textures -> FBO Destinations
        //
        fbo_src = (toggle) ? output1 : output0;
        fbo_dst = (toggle) ? output0 : output1;
    }
    while(i< count);

    return fbo_src; // NOTE:  It's NOT 'fbo_dst' ... the last toggle makes it 'fbo_src'
}

void drawTextEffects(int numQuads, const void *verts, const void* uvs, pxTextureRef t, const pxTextEffects* pe)
{
    if(pe->highlightEnabled)
    {
        // Draw HIGHLIGHT directly on FrameBuffer ... at back
        // float padding = (highlightPaddingLeft + highlightPaddingRight);
        context.pushState();

        // TODO: '0.575' is an Egregious MAGIC NUMBER
        pxMatrix4f m;
        m.translate(pe->highlightPaddingLeft, pe->highlightOffset + pe->highlightHeight * 0.575); // apply highlight offset
        context.setMatrix(m);

        rtLogInfo("drawTextEffects, highlight: w: %f, h: %f", pe->highlightWidth, pe->highlightHeight);
        float w = pe->highlightWidth - (pe->highlightPaddingLeft + pe->highlightPaddingRight);
        float h = pe->highlightHeight * 0.2; // TODO: '0.2' is an Egregious MAGIC NUMBER
        float lineWidth = 0;
        float fillColor[4];

        memcpy(&fillColor, pe->highlightColor, sizeof(fillColor));
        context.drawRect(w, h, lineWidth, fillColor, nullptr);
        context.popState();
    }

    if(pe->shadowEnabled)
    {
        pxContextFramebufferRef previousSurface;
        previousSurface = context.getCurrentFramebuffer();

        context.pushState();

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Setup Scratch buffers...

        float w = pe->shadowWidth * 1.2; // gResW;
        float h = pe->shadowHeight * 1.2; // gResH;

        pxContextFramebufferRef fbo0 = context.createFramebuffer(w,h);
        context.setFramebuffer(fbo0);
        context.clear(static_cast<int>(w), static_cast<int>(h) );

        pxMatrix4f m;
        m.translate(pe->shadowOffsetX, pe->shadowOffsetY);
        context.setMatrix(m);

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Draw the TEXT for shadow

        float shadowPM[4];
        premultiply(shadowPM, pe->shadowColor);

        // Draw 'solid' text to 'fbo0'
        gATextureShader->draw(w,h, gMatrix.data(), gAlpha, GL_TRIANGLES, numQuads, verts, uvs, t, shadowPM);

        float blurAmount   = pe->shadowBlur / 11.0;
        float kernelRadius = fmax(1, blurAmount /4);

        float blurFactor[] = {1.00, 1.25, 1.50, 1.75, 2.0};

        size_t len = sizeof(blurFactor) / sizeof(float);

        pxContextFramebufferRef input = fbo0;
        pxContextFramebufferRef output;

        for(size_t i =0; i<len; i++)
        {
            float f = blurFactor[i];

            const float amount = (f * blurAmount);
            const float radius = (f * kernelRadius);

            filterXYR blur_settings[] =   // [X Y] Radius
                    {
                            { amount,    0.0,   radius },
                            { 0.0,    amount,   radius },
                    };

            int count = sizeof(blur_settings) / sizeof(filterXYR);
            output = applyBlurSettings(input, pe, blur_settings, count);

            input = output;
        }
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        context.setFramebuffer(previousSurface);
        context.popState();
        // Draw final result of Shadow effects (on top of Higlight) to framebuffer
        context.drawImage(0,0, w,h, output->getTexture(), NULL);
    }
}

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


static void drawEffect(GLfloat x, GLfloat y, GLfloat w, GLfloat h, pxTextureRef t, shaderProgram *shader, void *options /* = nulptr*/)
{
  if(shader == NULL)
  {
    rtLogError("drawEffect() ... Shader is NULL !");
    return;
  }

  // args are tested at call site...

  const float verts[4][2] =
  {
    { x  , y   },
    { x+w, y   },
    { x  , y+h },
    { x+w, y+h }
  };

  float iw = static_cast<float>( t ? t->width()  : 1);
  float ih = static_cast<float>( t ? t->height() : 1);

  float tw = w/iw;
  float th = h/ih;

  float firstTextureY  = 1.0;
  float secondTextureY = static_cast<float>(1.0-th);

  const float uv[4][2] =
  {
    { 0,  firstTextureY  },
    { tw, firstTextureY  },
    { 0,  secondTextureY },
    { tw, secondTextureY }
  };

  // TODO - hacky, come back and clean up
  aLinearBlurShaderProgram *linearBlurShaderProgram = dynamic_cast<aLinearBlurShaderProgram*>(shader);
  if (linearBlurShaderProgram != NULL)
  {
    linearBlurShaderProgram->draw(gResW, gResH, gMatrix.data(), gAlpha, t, GL_TRIANGLE_STRIP, verts, (t ? uv : NULL), 4, options);
  }
  else
  {
    shader->draw(gResW, gResH, gMatrix.data(), gAlpha, t, GL_TRIANGLE_STRIP, verts, (t ? uv : NULL), 4);
  }
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
                             pxContextStretch xStretch,
                             pxContextStretch yStretch,
                             pxContextMaskOperation maskOp = pxContextMaskOperation::NORMAL)
{
  // args are tested at call site...

  float iw = static_cast<float>(texture->width());
  float ih = static_cast<float>(texture->height());

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
  case pxContextStretch::NONE:
    tw = w/iw;
    break;
  case pxContextStretch::STRETCH:
    tw = 1.0;
    break;
  case pxContextStretch::REPEAT:
    tw = w/iw;
    break;
  }

  float th;
  switch(yStretch) {
  case pxContextStretch::NONE:
    th = h/ih;
    break;
  case pxContextStretch::STRETCH:
    th = 1.0;
    break;
  case pxContextStretch::REPEAT:
#if 1 // PX_TEXTURE_ANCHOR_BOTTOM
    th = h/ih;
#else

    float temp = h/ih;
    th = ceil(temp);
    tb = 1.0f-(temp-floor(temp));
#endif
    break;
  }

  float firstTextureY  = 1.0;
  float secondTextureY = static_cast<float>(1.0-th);

  const float uv[4][2] =
  {
    { 0,  firstTextureY  },
    { tw, firstTextureY  },
    { 0,  secondTextureY },
    { tw, secondTextureY }
  };


  static float blackColor[4] = {0.0, 0.0, 0.0, 1.0};

  if (mask.getPtr() != NULL)
  {
    if (gTextureMaskedShader->draw(gResW,gResH,gMatrix.data(),gAlpha,4,verts,uv,texture,mask, maskOp) != PX_OK)
    {
      drawRect2(0, 0, iw, ih, blackColor); // DEFAULT - "Missing" - BLACK RECTANGLE
    }
  }
  else
  if (texture->getType() != PX_TEXTURE_ALPHA)
  {
    if (gTextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,4,verts,uv,texture,xStretch,yStretch) != PX_OK)
    {
      drawRect2(0, 0, iw, ih, blackColor); // DEFAULT - "Missing" - BLACK RECTANGLE
    }
  }
  else //PX_TEXTURE_ALPHA
  {
    float colorPM[4];
    premultiply(colorPM,color);

    if (gATextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_TRIANGLE_STRIP,4,verts,uv,texture,colorPM) != PX_OK)
    {
      drawRect2(0, 0, iw, ih, blackColor); // DEFAULT - "Missing" - BLACK RECTANGLE
    }
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

  float w2 = static_cast<float>(texture->width());
  float h2 = static_cast<float>(texture->height());

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

  gTextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,22,verts,uv,texture,pxContextStretch::NONE,pxContextStretch::NONE);
}

static void drawImage9Border2(GLfloat x, GLfloat y, GLfloat w, GLfloat h,
                       GLfloat borderX1, GLfloat borderY1, GLfloat borderX2, GLfloat borderY2,
                       GLfloat insetX1, GLfloat insetY1, GLfloat insetX2, GLfloat insetY2,
                       bool drawCenter, float* color,
                       pxTextureRef texture)
{
  // args are tested at call site...

  float ox1 = x;
  float ix1 = x+insetX1;
  float ix2 = x+w-insetX2;
  float ox2 = x+w;

  float oy1 = y;
  float iy1 = y+insetY1;
  float iy2 = y+h-insetY2;
  float oy2 = y+h;

  float w2 = static_cast<float>(texture->width());
  float h2 = static_cast<float>(texture->height());

  float ou1 = 0;
  float iu1 = borderX1/w2;
  float iu2 = (w2-borderX2)/w2;
  float ou2 = 1;

  float ov2 = 0;
  float iv2 = borderY1/h2;
  float iv1 = (h2-borderY2)/h2;
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

  const GLfloat verts[28][2] =
      {
          // border
          { ox1,oy2 },
          { ix1,oy2 },
          { ox1,iy2 },
          { ix1,iy2 },
          { ox1,iy1 },
          { ix1,iy1 },
          { ox1,oy1 },
          { ix1,oy1 },
          { ix1,oy1 },
          { ix1,iy1 },
          { ix2,oy1 },
          { ix2,iy1 },
          { ox2,oy1 },
          { ox2,iy1 },
          { ox2,iy1 },
          { ix2,iy1 },
          { ox2,iy2 },
          { ix2,iy2 },
          { ox2,oy2 },
          { ix2,oy2 },
          { ix2,oy2 },
          { ix2,iy2 },
          { ix1,oy2 },
          { ix1,iy2 },

          // center
          { ix1,iy2 },
          { ix2,iy2 },
          { ix1,iy1 },
          { ix2,iy1 },
      };

  const GLfloat uv[28][2] =
      {
          // border
          { ou1,ov1 },
          { iu1,ov1 },
          { ou1,iv1 },
          { iu1,iv1 },
          { ou1,iv2 },
          { iu1,iv2 },
          { ou1,ov2 },
          { iu1,ov2 },
          { iu1,ov2 },
          { iu1,iv2 },
          { iu2,ov2 },
          { iu2,iv2 },
          { ou2,ov2 },
          { ou2,iv2 },
          { ou2,iv2 },
          { iu2,iv2 },
          { ou2,iv1 },
          { iu2,iv1 },
          { ou2,ov1 },
          { iu2,ov1 },
          { iu2,ov1 },
          { iu2,iv1 },
          { iu1,ov1 },
          { iu1,iv1 },

          // center
          { iu1,iv1 },
          { iu2,iv1 },
          { iu1,iv2 },
          { iu2,iv2 },
      };

  float colorPM[4];
  premultiply(colorPM,color);

  gTextureBorderShader->draw(gResW,gResH,gMatrix.data(),gAlpha,drawCenter? 28 : 24,verts,uv,texture,pxContextStretch::NONE,pxContextStretch::NONE, colorPM);
}

bool gContextInit = false;

#define SAFE_DELETE(p)  if(p) { delete p; p = NULL; };

pxContext::~pxContext()
{
  SAFE_DELETE(gSolidShader);
  SAFE_DELETE(gATextureShader);
  SAFE_DELETE(gATextureBlurShader);
  SAFE_DELETE(gTextureShader);
  SAFE_DELETE(gTextureBorderShader);
  SAFE_DELETE(gTextureMaskedShader);
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

  SAFE_DELETE(gSolidShader);
  SAFE_DELETE(gATextureShader);
  SAFE_DELETE(gATextureBlurShader);
  SAFE_DELETE(gTextureShader);
  SAFE_DELETE(gTextureBorderShader);
  SAFE_DELETE(gTextureMaskedShader);

  gSolidShader = new solidShaderProgram();
  gSolidShader->initShader(vShaderText,fSolidShaderText);

  gATextureShader = new aTextureShaderProgram();
  gATextureShader->initShader(vShaderText,fATextureShaderText);

  gATextureBlurShader = new aLinearBlurShaderProgram();
  gATextureBlurShader->initShader(vShaderText,fLinearBlurShaderText);

  gTextureShader = new textureShaderProgram();
  gTextureShader->initShader(vShaderText,fTextureShaderText);

  gTextureBorderShader = new textureBorderShaderProgram();
  gTextureBorderShader->initShader(vShaderText,fTextureBorderShaderText);

  gTextureMaskedShader = new textureMaskedShaderProgram();
  gTextureMaskedShader->initShader(vShaderText,fTextureMaskedShaderText);

  glEnable(GL_BLEND);

  // assume non-premultiplied for now...
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
// non-premultiplied
//  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//  glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//  glUseProgram(program);

//  gprogram = program;

  rtValue val;
  if (RT_OK == rtSettings::instance()->value("enableTextureMemoryMonitoring", val))
  {
    mEnableTextureMemoryMonitoring = val.toString().compare("true") == 0;
  }
  if (RT_OK == rtSettings::instance()->value("textureMemoryLimitInMb", val))
  {
    setTextureMemoryLimit((int64_t)val.toInt32() * (int64_t)1024 * (int64_t)1024);
  }
  if (RT_OK == rtSettings::instance()->value("ejectTextureAge", val))
  {
    mEjectTextureAge = val.toUInt32();
  }
  if (mEnableTextureMemoryMonitoring)
  {
    rtLogDebug("texture memory limit set to %" PRId64 " bytes, threshold padding %" PRId64 " bytes",
      mTextureMemoryLimitInBytes, mTextureMemoryLimitThresholdPaddingInBytes);
  }

  if (RT_OK == rtSettings::instance()->value("targetTextureMemoryAfterCleanupInMb", val))
  {
    mTargetTextureMemoryAfterCleanupInBytes = (int64_t)val.toInt32() * (int64_t)1024 * (int64_t)1024;
  }
  else
  {
    mTargetTextureMemoryAfterCleanupInBytes = mTextureMemoryLimitInBytes - (mTextureMemoryLimitInBytes / (int64_t)3);
  }

  if (mTargetTextureMemoryAfterCleanupInBytes < 0)
  {
    mTargetTextureMemoryAfterCleanupInBytes = 0;
  }

  if (RT_OK == rtSettings::instance()->value("freeAllOffscreenTextureMemoryOnCleanup", val))
  {
    mFreeAllOffscreenTextureMemoryOnCleanup = val.toString().compare("true") == 0;
  }

  char const* textureLimitSetting = getenv("SPARK_TEXTURE_LIMIT_MB");
  if (textureLimitSetting)
  {
    int textureLimitInMb = atoi(textureLimitSetting);
    if (textureLimitInMb >= 0)
    {
      setTextureMemoryLimit((int64_t)textureLimitInMb * (int64_t)1024 * (int64_t)1024);
    }
  }

  char const* ejectTextureAgeSetting = getenv("SPARK_EJECT_TEXTURE_AGE");
  if (ejectTextureAgeSetting)
  {
    int ejectTextureAge = atoi(ejectTextureAgeSetting);
    if (ejectTextureAge >= 0)
    {
      mEjectTextureAge = (uint32_t)ejectTextureAge;
    }
  }
  char const* ejectTargetSetting = getenv("SPARK_EJECT_TEXTURE_MEMORY_TARGET_MB");
  if (ejectTargetSetting)
  {
    int targetTextureMemoryAfterCleanupInMb = atoi(ejectTargetSetting);
    if (targetTextureMemoryAfterCleanupInMb >= 0)
    {
      mTargetTextureMemoryAfterCleanupInBytes = (int64_t)targetTextureMemoryAfterCleanupInMb * (int64_t)1024 * (int64_t)1024;
    }
  }
  char const* ejectAllSetting = getenv("SPARK_EJECT_ALL_OFFSCREEN_TEXTURES");
  if (ejectAllSetting)
  {
    int ejectAll = atoi(ejectAllSetting);
    if (ejectAll > 0)
    {
      mFreeAllOffscreenTextureMemoryOnCleanup = true;
    }
    else
    {
      mFreeAllOffscreenTextureMemoryOnCleanup = false;
    }
  }

  rtLogInfo("texture memory target after cleanup: %" PRId64 " bytes.  Free all offscreen memory on cleanup: %s.  Eject texture age: %u",
            mTargetTextureMemoryAfterCleanupInBytes,
            mFreeAllOffscreenTextureMemoryOnCleanup ? "true":"false", mEjectTextureAge);

  char const* gcThrottleSetting = getenv("SPARK_GC_THROTTLE_SECS");
  if (gcThrottleSetting)
  {
    int gcThrottle = atoi(gcThrottleSetting);
    if (gcThrottle >= 0)
    {
      garbageCollectThrottleInSeconds = static_cast<double>(gcThrottle);
    }
  }

  rtLogWarn("initialization all complete");
  rtLogInfo("context garbage collect throttle set to %f seconds", garbageCollectThrottleInSeconds);

#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  defaultEglContext = eglGetCurrentContext();
  defaultEglDisplay = eglGetCurrentDisplay();
  defaultEglSurface = eglGetCurrentSurface(EGL_DRAW);
  rtLogDebug("current context in init: %p", defaultEglContext);
#endif //PX_PLATFORM_GENERIC_EGL || PX_PLATFORM_WAYLAND_EGL

  std::srand(unsigned (std::time(0)));
}

void pxContext::term()  // clean up statics
{
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

void pxContext::clear(int left, int top, int width, int height)
{
  if (left < 0)
  {
    left = 0;
  }
  if (top < 0)
  {
    top = 0;
  }
  if ((left+width) > gResW)
  {
    width = gResW - left;
  }
  if ((top+height) > gResH)
  {
    height = gResH - top;
  }
  int clearTop = gResH-top-height;

  glEnable(GL_SCISSOR_TEST);

  currentFramebuffer->setDirtyRectangle(left, clearTop, width, height);
  currentFramebuffer->enableDirtyRectangles(true);

  //map form screen to window coordinates
  glScissor(left, clearTop, width, height);
  glClear(GL_COLOR_BUFFER_BIT);
}

void pxContext::punchThrough(int left, int top, int width, int height)
{
  GLfloat priorColor[4];
  GLint priorBox[4];
  bool wasEnabled= glIsEnabled(GL_SCISSOR_TEST);
  glGetIntegerv( GL_SCISSOR_BOX, priorBox );
  glGetFloatv( GL_COLOR_CLEAR_VALUE, priorColor );

  glEnable( GL_SCISSOR_TEST );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );


  if (left < 0)
  {
    left = 0;
  }
  if (top < 0)
  {
    top = 0;
  }
  if ((left+width) > gResW)
  {
    width = gResW - left;
  }
  if ((top+height) > gResH)
  {
    height = gResH - top;
  }
  int clearTop = gResH-top-height;

  glEnable(GL_SCISSOR_TEST);

  //map form screen to window coordinates
  glScissor(left, clearTop, width, height);
  glClear(GL_COLOR_BUFFER_BIT);

  glClearColor( priorColor[0], priorColor[1], priorColor[2], priorColor[3] );
  if ( wasEnabled )
  {
    glScissor( priorBox[0], priorBox[1], priorBox[2], priorBox[3] );
  }
  else
  {
    glDisable( GL_SCISSOR_TEST );
  }
}

void pxContext::enableClipping(bool enable)
{
  if (enable)
  {
    glEnable(GL_SCISSOR_TEST);
  }
  else
  {
    glDisable(GL_SCISSOR_TEST);
  }
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

pxContextFramebufferRef pxContext::createFramebuffer(int width, int height, bool antiAliasing, bool alphaOnly, bool depthBuffer)
{
  pxContextFramebuffer* fbo = new pxContextFramebuffer();
  pxFBOTexture* fboTexture = new pxFBOTexture(antiAliasing, alphaOnly, depthBuffer);
  pxTextureRef texture = fboTexture;

  fboTexture->createFboTexture(width, height);

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
  currentGLProgram = -1; //NB: Force a re-bind of GL program

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
    gFlipRendering = currentFramebuffer->flipRenderingEnabled();

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
  gFlipRendering = currentFramebuffer->flipRenderingEnabled();

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
   // rtLogDebug("\n drawRect() - TRANSPARENT");
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
    float half = lineWidth;
    drawRect2(half, half, w-2*lineWidth, h-2*lineWidth, fillColor);
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

  texture->setLastRenderTick(gRenderTick);

  drawImage92(0, 0, w, h, x1, y1, x2, y2, texture);
}

void pxContext::drawImage9Border(float w, float h,
                  float bx1, float by1, float bx2, float by2,
                  float ix1, float iy1, float ix2, float iy2,
                  bool drawCenter, float* color,
                  pxTextureRef texture)
{
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

  texture->setLastRenderTick(gRenderTick);

  drawImage9Border2(0, 0, w, h, bx1, by1, bx2, by2, ix1, iy1, ix2, iy2, drawCenter, color, texture);
}

// convenience method
void pxContext::drawImageMasked(float x, float y, float w, float h,
                                pxContextMaskOperation maskOp,
                                pxTextureRef t, pxTextureRef mask)
{
  this->drawImage(x, y, w, h, t , mask,
                    /* useTextureDimsAlways = */ true, /*color = */ NULL,      // DEFAULT
                    /*             stretchX = */ pxContextStretch::STRETCH,  // DEFAULT
                    /*             stretchY = */ pxContextStretch::STRETCH,  // DEFAULT
                    /*      downscaleSmooth = */ false,                        // DEFAULT
                                                 maskOp                        // PARAMETER
                    );
};

void pxContext::drawImage(float x, float y, float w, float h,
                          pxTextureRef t, pxTextureRef mask,
                          bool useTextureDimsAlways               /* = true */,
                          float* color,                           /* = NULL */
                          pxContextStretch stretchX, /* = pxContextStretch::STRETCH, */
                          pxContextStretch stretchY, /* = pxContextStretch::STRETCH, */
                          bool downscaleSmooth                    /* = false */,
                          pxContextMaskOperation maskOp     /* = pxContextMaskOperation::NORMAL */ )
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

  t->setLastRenderTick(gRenderTick);
  t->setDownscaleSmooth(downscaleSmooth);

  if (mask.getPtr() != NULL)
  {
    mask->setLastRenderTick(gRenderTick);
  }

  if (stretchX < pxContextStretch::NONE || stretchX > pxContextStretch::REPEAT)
  {
    stretchX = pxContextStretch::NONE;
  }

  if (stretchY < pxContextStretch::NONE || stretchY > pxContextStretch::REPEAT)
  {
    stretchY = pxContextStretch::NONE;
  }

  float black[4] = {0,0,0,1};
  drawImageTexture(x, y, w, h, t, mask, useTextureDimsAlways,
                   color? color : black, stretchX, stretchY, maskOp);
}

#ifdef PXSCENE_FONT_ATLAS
void pxContext::drawTexturedQuads(int numQuads, const void *verts, const void* uvs,
                          pxTextureRef t, float* color)
{
#ifdef DEBUG_SKIP_IMAGE
#warning "DEBUG_SKIP_IMAGE enabled ... Skipping "
  return;
#endif

  // TRANSPARENT
  if(gAlpha == 0.0)
  {
    return;
  }

  // TEXTURELESS
  if (t.getPtr() == NULL)
  {
    return;
  }

  t->setLastRenderTick(gRenderTick);

  float colorPM[4];
  premultiply(colorPM,color);

  // - - - - - - - - - - - - - - - - - - - -
  // Draw the TEXT quads
  gATextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_TRIANGLES,6*numQuads,verts,uvs,t,colorPM);
  // - - - - - - - - - - - - - - - - - - - -
}

void
pxContext::drawTexturedQuadsWithEffects(int numQuads, const void *verts, const void *uvs, pxTextureRef t, float *color,
                                        const pxTextEffects *pe)
{
#ifdef DEBUG_SKIP_IMAGE
    #warning "DEBUG_SKIP_IMAGE enabled ... Skipping "
  return;
#endif

    // TRANSPARENT
    if(gAlpha == 0.0)
    {
        return;
    }

    // TEXTURELESS
    if (t.getPtr() == NULL)
    {
        return;
    }

    t->setLastRenderTick(gRenderTick);

    // Shadow / Highlight draw
    //
    drawTextEffects( 6*numQuads, verts, uvs, t, pe);

    float colorPM[4];
    premultiply(colorPM,color);

    // - - - - - - - - - - - - - - - - - - - -
    // Draw the TEXT quads
    gATextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_TRIANGLES,6*numQuads,verts,uvs,t,colorPM);
    // - - - - - - - - - - - - - - - - - - - -
}

#endif

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
  //  rtLogError("cannot drawDiagRect() - width/height/gAlpha cannot be Zero.");
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

// convenience method
void pxContext::drawEffect(float x, float y, float w, float h, pxTextureRef t, shaderProgram *shader, void *options /* = nullptr */)
{
  ::drawEffect(x, y, w, h, t, shader, options);
};

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

pxSharedContextRef pxContext::createSharedContext(bool depthBuffer)
{
  pxSharedContext* sharedContext = new pxSharedContext(depthBuffer);
  return sharedContext;
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
    outX = static_cast<int> (positionCoords.x());
    outY = static_cast<int> (positionCoords.y());
  }
  else
  {
    outX = static_cast<int> (positionCoords.x() / positionCoords.w());
    outY = static_cast<int> (positionCoords.y() / positionCoords.w());
  }
}

void pxContext::mapToScreenCoordinates(pxMatrix4f& m, float inX, float inY, int &outX, int &outY)
{
  pxVector4f positionVector(inX, inY, 0, 1);
  pxVector4f positionCoords = m.multiply(positionVector);

  if (positionCoords.w() == 0)
  {
    outX = static_cast<int> (positionCoords.x());
    outY = static_cast<int> (positionCoords.y());
  }
  else
  {
    outX = static_cast<int> (positionCoords.x() / positionCoords.w());
    outY = static_cast<int> (positionCoords.y() / positionCoords.w());
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

void pxContext::adjustCurrentTextureMemorySize(int64_t changeInBytes, bool allowGarbageCollect)
{
  lockContext();
  mCurrentTextureMemorySizeInBytes += changeInBytes;
  if (mCurrentTextureMemorySizeInBytes < 0)
  {
    mCurrentTextureMemorySizeInBytes = 0;
  }
  int64_t currentTextureMemorySize = mCurrentTextureMemorySizeInBytes;
  int64_t maxTextureMemoryInBytes = mTextureMemoryLimitInBytes;

  unlockContext();
  //rtLogDebug("the current texture size: %" PRId64 ".", currentTextureMemorySize);
  if (mEnableTextureMemoryMonitoring && allowGarbageCollect && changeInBytes > 0 && currentTextureMemorySize > maxTextureMemoryInBytes)
  {
    double timeSinceGc = pxSeconds() - lastContextGarbageCollectTime;
    if (timeSinceGc >= garbageCollectThrottleInSeconds)
    {
      rtLogDebug("the texture size is too large: %" PRId64 ".  doing a garbage collect!!!\n", currentTextureMemorySize);
#ifdef RUNINMAIN
      script.collectGarbage();
#else
      uv_async_send(&gcTrigger);
#endif
      lastContextGarbageCollectTime = pxSeconds();
    }
  }
}

void pxContext::setTextureMemoryLimit(int64_t textureMemoryLimitInBytes)
{
  mTextureMemoryLimitInBytes = textureMemoryLimitInBytes;
}

bool pxContext::isTextureSpaceAvailable(pxTextureRef texture, bool allowGarbageCollect, int32_t bytesPerPixel)
{
  if (!mEnableTextureMemoryMonitoring)
    return true;

  int64_t textureSize = ((int64_t)(texture->width())*(int64_t)(texture->height())*(int64_t)bytesPerPixel);
  lockContext();
  int64_t currentTextureMemorySize = mCurrentTextureMemorySizeInBytes;
  int64_t maxTextureMemoryInBytes = mTextureMemoryLimitInBytes;
  //rtLogDebug("current size %" PRId64 " limit %" PRId64 ".", currentTextureMemorySize, maxTextureMemoryInBytes);
  unlockContext();
  if ((textureSize + currentTextureMemorySize) >
             (maxTextureMemoryInBytes  + mTextureMemoryLimitThresholdPaddingInBytes))
  {
    if (allowGarbageCollect)
    {
      double timeSinceGc = pxSeconds() - lastContextGarbageCollectTime;
      if (timeSinceGc >= garbageCollectThrottleInSeconds)
      {
#ifdef RUNINMAIN
        script.collectGarbage();
#else
        uv_async_send(&gcTrigger);
#endif
        lastContextGarbageCollectTime = pxSeconds();
        lockContext();
        currentTextureMemorySize = mCurrentTextureMemorySizeInBytes;
        unlockContext();
        return ((textureSize + currentTextureMemorySize) <=
                (maxTextureMemoryInBytes + mTextureMemoryLimitThresholdPaddingInBytes));
      }
    }
    return false;
  }
  else if (allowGarbageCollect && (textureSize + currentTextureMemorySize) > maxTextureMemoryInBytes)
  {
    int64_t textureMemoryNeeded = textureMemoryOverflow(texture);
    context.ejectTextureMemory(textureMemoryNeeded);
    lockContext();
    currentTextureMemorySize = mCurrentTextureMemorySizeInBytes;
    unlockContext();
    if ((textureSize + currentTextureMemorySize) > maxTextureMemoryInBytes)
    {
      double timeSinceGc = pxSeconds() - lastContextGarbageCollectTime;
      if (timeSinceGc >= garbageCollectThrottleInSeconds)
      {
#ifdef RUNINMAIN
        rtLogInfo("gc for texture memory");
        script.collectGarbage();
#else
        uv_async_send(&gcTrigger);
#endif
        lastContextGarbageCollectTime = pxSeconds();
      }
    }
    else
    {
      rtLogInfo("texture memory freed by clearing offscreen");
    }
  }
  return true;
}

int64_t pxContext::currentTextureMemoryUsageInBytes()
{
  return mCurrentTextureMemorySizeInBytes;
}

int64_t pxContext::textureMemoryOverflow(pxTextureRef texture)
{
  int64_t textureSize = (((int64_t)texture->width())*((int64_t)texture->height())*4);
  int64_t currentTextureMemorySize = 0;
  lockContext();
  currentTextureMemorySize = mCurrentTextureMemorySizeInBytes;
  unlockContext();
  int64_t availableBytes = mTextureMemoryLimitInBytes - currentTextureMemorySize;
  if (textureSize > availableBytes)
  {
    return (textureSize - availableBytes);
  }
  return 0;
}

int64_t pxContext::ejectTextureMemory(int64_t bytesRequested, bool forceEject)
{
#ifdef ENABLE_LRU_TEXTURE_EJECTION
  if (!mEnableTextureMemoryMonitoring)
    return 0;

  int64_t beforeTextureMemoryUsage = context.currentTextureMemoryUsageInBytes();
  if (!forceEject)
  {
    ejectNotRecentlyUsedTextureMemory(bytesRequested, mTargetTextureMemoryAfterCleanupInBytes,
                                      mFreeAllOffscreenTextureMemoryOnCleanup, mEjectTextureAge);
  }
  else
  {
    ejectNotRecentlyUsedTextureMemory(bytesRequested, mTargetTextureMemoryAfterCleanupInBytes,
                                      mFreeAllOffscreenTextureMemoryOnCleanup, 0);
  }
  int64_t afterTextureMemoryUsage = context.currentTextureMemoryUsageInBytes();
  return (beforeTextureMemoryUsage-afterTextureMemoryUsage);
#else
  (void)bytesRequested;
  (void)forceEject;
  return 0;
#endif //ENABLE_LRU_TEXTURE_EJECTION
}

pxError pxContext::setEjectTextureAge(uint32_t age)
{
  mEjectTextureAge = age;
  return PX_OK;
}

void pxContext::updateRenderTick()
{
  {
    rtMutexLockGuard renderTickMutexGuard(gRenderTickMutex);
    gRenderTick++;
  }
}

