#include "rtDefs.h"
#include "rtLog.h"
#include "pxContext.h"


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
#include <GL/glext.h>
#endif //PX_PLATFORM_WAYLAND_EGL
#endif

#define PX_TEXTURE_MIN_FILTER GL_LINEAR
#define PX_TEXTURE_MAG_FILTER GL_LINEAR

#if 0
static GLuint textureId1;
#endif

pxContextSurfaceNativeDesc defaultContextSurface;
pxContextSurfaceNativeDesc* currentContextSurface = &defaultContextSurface;

pxTextureRef defaultRenderSurface;
pxTextureRef currentRenderSurface = defaultRenderSurface;

// TODO get rid of this global crap

static GLuint gprogram;

static GLint u_matrix = -1;
static GLint u_alpha = -1;
static GLint u_resolution = -1;
static GLint u_texture = -1;
static GLint u_mask = -1;
static GLint u_enablemask = 0;
static GLint u_alphatexture = -1;
static GLint u_color = -1;
static GLint attr_pos = 0, attr_uv = 2;

static int gResW, gResH;
static pxMatrix4f gMatrix;
static float gAlpha;

int gAlphaTexture = 1;
bool gEnableMask = false;

// assume premultiplied
static const char *fSolidShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;\n"
#endif
  "uniform float u_alpha;\n"
  "uniform vec4 a_color;\n"
  "void main()"
  "{\n"
  "  gl_FragColor = a_color*u_alpha;"
  "}\n";

// assume premultiplied
static const char *fTextureShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;\n"
#endif
  "uniform sampler2D s_texture;\n"
  "uniform float u_alpha;\n"
  "varying vec2 v_uv;\n"
  "void main()\n"
  "{\n"
  "  gl_FragColor = texture2D(s_texture, v_uv) * u_alpha;"
  "}\n";

// assume premultiplied
static const char *fTextureMaskedShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;\n"
#endif
  "uniform sampler2D s_texture;\n"
  "uniform sampler2D s_mask;\n"
  "uniform float u_alpha;\n"
  "varying vec2 v_uv;\n"
  "void main() {\n"
  "  float a = u_alpha * texture2D(s_mask, v_uv).a;\n"
  "  gl_FragColor = texture2D(s_texture, v_uv) * a;\n"
  "}\n";

// assume premultiplied
static const char *fATextureShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;\n"
#endif
  "uniform sampler2D s_texture;\n"
  "uniform float u_alpha;\n"
  "uniform vec4 a_color;\n"
  "varying vec2 v_uv;\n"
  "void main() {\n"
  "  float a = u_alpha * texture2D(s_texture, v_uv).a;"
  "  gl_FragColor = a_color*a;"
  "}\n";

//////////////////
#if 1 
// assume premultiplied
static const char *fMondoShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;\n"
#endif
  "uniform float u_alphatexture;\n"
  "uniform float u_alpha;\n"
  "uniform vec4 a_color;\n"
  "uniform sampler2D s_texture;\n"
  "uniform sampler2D s_mask;\n"
  "uniform int u_enablemask;\n"
//  "uniform int u_enablepremultipliedalpha;\n"
//  "uniform int u_flipmaskcoords;\n"
  "varying vec2 v_uv;\n"
  "void main() {\n"
#if 1
  "if (u_alphatexture < 1.0) {"
  // solid color
  " gl_FragColor = a_color;"
  "} else "
  "if (u_alphatexture < 2.0) {\n"
  // image
  "  vec4 textureColor = texture2D(s_texture, v_uv);\n"
  "  if (u_enablemask > 0) {\n"
  "    vec4 maskColor = texture2D(s_mask, v_uv);\n"
  "    textureColor *= maskColor.a;\n" 
  "  }\n"
  "  gl_FragColor = textureColor;\n"
  "} else {\n"
  // text
  "  gl_FragColor = a_color * texture2D(s_texture, v_uv).a;"
  "}\n"
  "gl_FragColor *= u_alpha;\n"
#else
  "gl_FragColor = vec4(1,1,1,1);"
#endif
  "}\n";
#endif


static const char *vShaderText =
  "uniform vec2 u_resolution;\n"
  "uniform mat4 amymatrix;\n"
  "attribute vec2 pos;\n"
  "attribute vec2 uv;\n"
  "varying vec2 v_uv;\n"
  "void main() {\n"
  // map from "pixel coordinates"
  " vec4 p = amymatrix * vec4(pos, 0, 1);\n"
  " vec4 zeroToOne = p / vec4(u_resolution, u_resolution.x, 1);\n"
  " vec4 zeroToTwo = zeroToOne * vec4(2.0, 2.0, 1, 1);\n"
  " vec4 clipSpace = zeroToTwo - vec4(1.0, 1.0, 0, 0);\n"
  " clipSpace.w = 1.0+clipSpace.z;\n"
  //" clipSpace = clipSpace/vec4(1.0+clipSpace.z, 1.0+clipSpace.z, 1, 1);\n"
  " gl_Position =  clipSpace * vec4(1, -1, 1, 1);\n"
  "v_uv = uv;\n"
  "}\n";

class pxFBOTexture : public pxTexture
{
public:
  pxFBOTexture() : mWidth(0), mHeight(0), mFramebufferId(0), mTextureId(0)
  {
    mTextureType = PX_TEXTURE_FRAME_BUFFER;
  }

  ~pxFBOTexture() { deleteTexture(); }

  void createTexture(int width, int height)
  {
    if (mFramebufferId != 0 && mTextureId != 0)
    {
      deleteTexture();
    }
    
    mWidth = width;
    mHeight = height;

    glGenFramebuffers(1, &mFramebufferId);
    glGenTextures(1, &mTextureId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                 width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1f(u_alphatexture, 1.0);
    gAlphaTexture = 1;
  }
  
  pxError resizeTexture(int width, int height)
  { 
    if (mWidth != width || mHeight != height || 
        mFramebufferId == 0 || mTextureId == 0)
    {
      createTexture(width, height);
      return PX_OK;
    }

    // TODO crashing in glTexSubImage2d in osx... 
    #ifdef __APPLE__
    return PX_OK;
    #endif

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mTextureId);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                 width, height, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1f(u_alphatexture, 1.0);
    gAlphaTexture = 1;
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
    }
    
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
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glViewport ( 0, 0, mWidth, mHeight);
//    glUniform2f(u_resolution, mWidth, mHeight);
    gResW = mWidth;
    gResH = mHeight;
    
    return PX_OK;
  }

  // TODO get rid of pxError
  // TODO get rid of bindTexture() and bindTextureAsMask()
  virtual pxError bindTexture(int tLoc)
  {
    if (mFramebufferId == 0 || mTextureId == 0)
    {
      return PX_NOTINITIALIZED;
    }

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mTextureId);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
//    glUniform1i(u_texture, 3);
    glUniform1i(tLoc,3);
    glUniform1f(u_alphatexture, 1.0);
    gAlphaTexture = 1;

    return PX_OK;
  }

#if 0
  virtual pxError getGLTextureName(GLuint& name)
  {
    if (mFramebufferId == 0 || mTextureId == 0)
    {
      return PX_NOTINITIALIZED;
    }

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mTextureId);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glUniform1i(u_texture, 3);
    glUniform1f(u_alphatexture, 1.0);
    gAlphaTexture = 1;

    return PX_OK;    
  }
#endif
  
  virtual pxError bindTextureAsMask(int mLoc)
  {
    if (mFramebufferId == 0 || mTextureId == 0)
    {
      return PX_NOTINITIALIZED;
    }

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

//    glUniform1i(u_mask, 2);
    glUniform1i(mLoc, 2);
    glUniform1i(u_enablemask, 1);
    gEnableMask = true;
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
};

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
    mOffscreen.setUpsideDown(true);
    o.blit(mOffscreen);
    
#if 1
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
#endif

    mInitialized = true;
  }
  
  virtual pxError deleteTexture()
  {
    rtLogInfo("pxTextureOffscreen::deleteTexture()");
    if (mTextureName) glDeleteTextures(1, &mTextureName);
    mInitialized = false;
    return PX_OK;
  }

  virtual pxError bindTexture(int tLoc)
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }
    

    glActiveTexture(GL_TEXTURE0); 


// TODO would be nice to do the upload in createTexture but right now it's getting called on wrong thread
    if (!mTextureUploaded)  
    {
      glGenTextures(1, &mTextureName);
      glBindTexture(GL_TEXTURE_2D, mTextureName);    
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                   mOffscreen.width(), mOffscreen.height(), 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, mOffscreen.base());
      mTextureUploaded = true;
    }
    else
      glBindTexture(GL_TEXTURE_2D, mTextureName);    

//    glUniform1i(u_texture, 0);
    glUniform1i(tLoc, 0);
    glUniform1f(u_alphatexture, 1.0);
    gAlphaTexture = 1;
    return PX_OK;
  }
  
  virtual pxError bindTextureAsMask(int mLoc)
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }
    
    glActiveTexture(GL_TEXTURE2);

    if (!mTextureUploaded)  
    {
      glGenTextures(1, &mTextureName);
      glBindTexture(GL_TEXTURE_2D, mTextureName);    
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, PX_TEXTURE_MIN_FILTER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, PX_TEXTURE_MAG_FILTER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                   mOffscreen.width(), mOffscreen.height(), 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, mOffscreen.base());
      mTextureUploaded = true;
    }
    else
      glBindTexture(GL_TEXTURE_2D, mTextureName);    

    glUniform1i(mLoc, 2);
    glUniform1i(u_enablemask, 1);
    gEnableMask = true;

    return PX_OK;
  }
  
  virtual pxError getOffscreen(pxOffscreen& o)
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }
    o.init(mOffscreen.width(), mOffscreen.height());
    mOffscreen.blit(o);
    return PX_OK;
  }

  virtual int width() { return mOffscreen.width(); }
  virtual int height() { return mOffscreen.height(); }
  
private:
  pxOffscreen mOffscreen;
  bool mInitialized;
  GLuint mTextureName;
//  int mTextureUnit;
  bool mTextureUploaded;
};

class pxTextureAlpha : public pxTexture
{
public:
  pxTextureAlpha() : mDrawWidth(0.0), mDrawHeight (0.0), mImageWidth(0.0),
                     mImageHeight(0.0), mTextureId(0), mInitialized(false)
  {
    mTextureType = PX_TEXTURE_ALPHA;
  }

  pxTextureAlpha(float w, float h, float iw, float ih, void* buffer) 
    : mDrawWidth(w), mDrawHeight (h), mImageWidth(iw),
      mImageHeight(ih), mTextureId(0), mInitialized(false), mBuffer(NULL)
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
      free(mBuffer);
    deleteTexture(); 
  }
  
  void createTexture(float w, float h, float iw, float ih)
  {
    if (mTextureId != 0)
    {
      deleteTexture();
    }
    glGenTextures(1, &mTextureId);
    
    mDrawWidth = w;
    mDrawHeight = h;
    mImageWidth = iw;
    mImageHeight = ih;
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
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
    mInitialized = true;
  }
  
  virtual pxError deleteTexture()
  {
    if (mTextureId != 0)
    {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
    }
    mInitialized = false;
    return PX_OK;
  }
  
  virtual pxError bindTexture(int tLoc)
  {
    // TODO Moved to here because of js threading issues
    if (!mInitialized) createTexture(mDrawWidth,mDrawHeight,mImageWidth,mImageHeight);
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTextureId);    
    glUniform1i(tLoc, 1);

    glUniform1f(u_alphatexture, 2.0);
    gAlphaTexture = 2;

    return PX_OK;
  }
  
  virtual pxError bindTextureAsMask(int mLoc)
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
//TODO Hmmm this looks wrong
    glUniform1i(u_texture, 1);
    glUniform1f(u_alphatexture, 2.0);
    gAlphaTexture = 2;

//    glUniform4fv(u_color, 1, mColor);
    
    glUniform1i(mLoc, 2);
    glUniform1i(u_enablemask, 1);
    gEnableMask = true;
    
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

  virtual int width() { return mDrawWidth; }
  virtual int height() { return mDrawHeight; }
  
private:
  float mDrawWidth;
  float mDrawHeight;
  float mImageWidth;
  float mImageHeight;
  GLuint mTextureId;
  bool mInitialized;
  void* mBuffer;
//  bool mUploaded;
};

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
    
    //Provide the infolog in whatever manor you deem best.
    rtLogWarn("%s", &errorLog[0]);
    //Exit with failure.
    glDeleteShader(fragShader); //Don't leak the shader.

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

class shaderProgram
{
public:
  virtual void init(const char* v, const char* f)
  {
    printf("shaderProgram\n");
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
    glUseProgram(mProgram);
  }

protected:
  // Override to do uniform lookups
  virtual void prelink() {}
  virtual void postlink() {}

  GLuint mProgram;
};

class solidShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    printf("prelink\n");
    mPosLoc = 0;
    mUVLoc = 2;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc, "uv");
  }

  virtual void postlink()
  {
    printf("postlink\n");
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc = getUniformLocation("amymatrix");
    mColorLoc = getUniformLocation("a_color");
    mAlphaLoc = getUniformLocation("u_alpha");
  }

public:
  void draw(int resW, int resH, float* matrix, float alpha, 
            GLenum mode,
            const void* pos, 
            int count,
            const float* color)
  {
    use();
    glUniform2f(mResolutionLoc, resW, resH);
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1f(mAlphaLoc, alpha);
    glUniform4fv(mColorLoc, 1, color);

    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glEnableVertexAttribArray(attr_pos);
    glDrawArrays(mode, 0, count);
    glDisableVertexAttribArray(attr_pos);
    glUseProgram(gprogram);
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mColorLoc;
  GLint mAlphaLoc;
};


solidShaderProgram *gSolidShader;

class aTextureShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    printf("prelink\n");
    mPosLoc = 0;
    mUVLoc = 2;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc, "uv");
  }

  virtual void postlink()
  {
    printf("postlink\n");
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc = getUniformLocation("amymatrix");
    mColorLoc = getUniformLocation("a_color");
    mAlphaLoc = getUniformLocation("u_alpha");
    mTextureLoc = getUniformLocation("s_texture");
  }

public:
  void draw(int resW, int resH, float* matrix, float alpha, 
            int count,
            const void* pos, 
            const void* uv,
            pxTextureRef texture,
            const float* color)
  {
    use();
    glUniform2f(mResolutionLoc, resW, resH);
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1f(mAlphaLoc, alpha);
    glUniform4fv(mColorLoc, 1, color);
    
    texture->bindTexture(mTextureLoc);

    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_uv);
    
    // TODO this should go away
    glUseProgram(gprogram);
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mColorLoc;
  GLint mAlphaLoc;

  GLint mTextureLoc;
};


aTextureShaderProgram *gATextureShader;

class textureShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    printf("prelink\n");
    mPosLoc = 0;
    mUVLoc = 2;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc, "uv");
  }

  virtual void postlink()
  {
    printf("postlink\n");
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc = getUniformLocation("amymatrix");
    mAlphaLoc = getUniformLocation("u_alpha");
    mTextureLoc = getUniformLocation("s_texture");
  }

public:
  void draw(int resW, int resH, float* matrix, float alpha, 
            int count,            
            const void* pos, const void* uv,
            pxTextureRef texture,
            int32_t xStretch, int32_t yStretch)
  {
    use();
    glUniform2f(mResolutionLoc, resW, resH);
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1f(mAlphaLoc, alpha);

//    printf("mAlphaLoc %d, %f\n", mAlphaLoc, alpha);

    texture->bindTexture(mTextureLoc);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 
		    (xStretch==PX_REPEAT)?GL_REPEAT:GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 
		    (yStretch==PX_REPEAT)?GL_REPEAT:GL_CLAMP_TO_EDGE);

    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_uv);

    // TODO This should go away
    glUseProgram(gprogram);
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mAlphaLoc;

  GLint mTextureLoc;
};


textureShaderProgram *gTextureShader;

class textureMaskedShaderProgram: public shaderProgram
{
protected:
  virtual void prelink()
  {
    printf("prelink\n");
    mPosLoc = 0;
    mUVLoc = 2;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc, "uv");
  }

  virtual void postlink()
  {
    printf("postlink\n");
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc = getUniformLocation("amymatrix");
    mAlphaLoc = getUniformLocation("u_alpha");
    mTextureLoc = getUniformLocation("s_texture");
    mMaskLoc = getUniformLocation("s_mask");
  }

public:
  void draw(int resW, int resH, float* matrix, float alpha, 
            int count,            
            const void* pos, 
            const void* uv,
            pxTextureRef texture,
            pxTextureRef mask)
  {
    use();
    glUniform2f(mResolutionLoc, resW, resH);
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1f(mAlphaLoc, alpha);
    
#if 1
  texture->bindTexture(mTextureLoc);
#endif
  
  if (mask.getPtr() != NULL)
  {
    mask->bindTextureAsMask(mMaskLoc);
  }


    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_uv);

    // TODO This should go away
    glUseProgram(gprogram);
  }

private:
  GLint mResolutionLoc;
  GLint mMatrixLoc;

  GLint mPosLoc;
  GLint mUVLoc;

  GLint mColorLoc;
  GLint mAlphaLoc;

  GLint mTextureLoc;
  GLint mMaskLoc;
};

textureMaskedShaderProgram *gTextureMaskedShader;

static void drawRect2(GLfloat x, GLfloat y, GLfloat w, GLfloat h, const float* c)
{
  const float verts[4][2] =
  {
    { x,y },
    {  x+w, y },
    {  x,  y+h },
    {  x+w, y+h }
  };
  
  float colorPM[4];
  memcpy(&colorPM, c, sizeof(colorPM));
  colorPM[0] *= colorPM[3];
  colorPM[1] *= colorPM[3];
  colorPM[2] *= colorPM[3];
  
  gSolidShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_TRIANGLE_STRIP,verts,4,colorPM); 
}


static void drawRectOutline(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat lw, const float* c)
{
  float half = lw/2;
  float ox1  = x-half;
  float ix1  = x+half;
  float ox2  = x+w+half;
  float ix2  = x+w-half;
  float oy1  = y-half;
  float iy1  = y+half;
  float oy2  = y+h+half;
  float iy2  = y+h-half;
  
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
  memcpy(&colorPM, c, sizeof(colorPM));
  colorPM[0] *= colorPM[3];
  colorPM[1] *= colorPM[3];
  colorPM[2] *= colorPM[3];
  
  gSolidShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_TRIANGLE_STRIP,verts,10,colorPM); 
}

static void drawImageTexture(float x, float y, float w, float h, pxTextureRef texture,
                             pxTextureRef mask, pxStretch xStretch, pxStretch yStretch, float* color)
{

  if (texture.getPtr() == NULL)
  {
    return;
  }

  float iw = texture->width();
  float ih = texture->height();
  
  if (xStretch == PX_NONE)
    w = iw;
  if (yStretch == PX_NONE)
    h = ih;

  const float verts[4][2] = 
  {

    { x,y },
    {  x+w, y },
    {  x,  y+h },
    {  x+w, y+h }
  };

  float tw;
  switch(xStretch) {
  case PX_NONE:
  case PX_STRETCH:
    tw = 1.0;
    break;
  case PX_REPEAT:
    tw = w/iw;
    break;
  }

  float th;
  switch(yStretch) {
  case PX_NONE:
  case PX_STRETCH:
    th = 1.0;
    break;
  case PX_REPEAT:
    th = h/ih;
    break;
  }
  
  float firstTextureY = th;
  float secondTextureY = 0;

  const float uv[4][2] = {
    { 0,  firstTextureY },
    { tw, firstTextureY },
    { 0,  secondTextureY },
    { tw, secondTextureY }
  };
  
  float colorPM[4];
  memcpy(&colorPM, color, sizeof(colorPM));
  colorPM[0] *= colorPM[3];
  colorPM[1] *= colorPM[3];
  colorPM[2] *= colorPM[3];


  if (mask.getPtr() == NULL && texture->getType() != PX_TEXTURE_ALPHA)
  {
    gTextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,4,verts,uv,texture,xStretch,yStretch);
  }
  else if (mask.getPtr() == NULL && texture->getType() == PX_TEXTURE_ALPHA)
  {
    gATextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,4,verts,uv,texture,colorPM);
  }
  else if (mask.getPtr() != NULL)
  {
    gTextureMaskedShader->draw(gResW,gResH,gMatrix.data(),gAlpha,4,verts,uv,texture,mask);
  }
  else
  {
    rtLogError("Unhandled case");
  }
}

static void drawImage92(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat x1, GLfloat y1, GLfloat x2, 
                        GLfloat y2, pxTextureRef texture)
{


  glUseProgram(gprogram);
  if (texture.getPtr() == NULL)
    return;


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

  gTextureShader->draw(gResW,gResH,gMatrix.data(),gAlpha,22,verts,uv,texture,PX_NONE,PX_NONE);
}

void pxContext::init()
{
#if !defined(__APPLE__) && !defined(PX_PLATFORM_WAYLAND_EGL) && !defined(PX_PLATFORM_GENERIC_EGL)

  GLenum err = glewInit();

  if (err != GLEW_OK)
  {
    rtLogError("failed to initialize glew");
    exit(1); // or handle the error in a nicer way
  }

  if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
  {
    rtLogError("invalid glew version");
    exit(1); // or handle the error in a nicer way
  }
#endif

  glClearColor(0, 0, 0, 0);

  gSolidShader = new solidShaderProgram();
  gSolidShader->init(vShaderText,fSolidShaderText);

  gATextureShader = new aTextureShaderProgram();
  gATextureShader->init(vShaderText,fATextureShaderText);

  gTextureShader = new textureShaderProgram();
  gTextureShader->init(vShaderText,fTextureShaderText);

  gTextureMaskedShader = new textureMaskedShaderProgram();
  gTextureMaskedShader->init(vShaderText,fTextureMaskedShaderText);

  printf("before\n");
  GLuint program = createShaderProgram(vShaderText, fMondoShaderText);
  printf("after\n");

  // has to happen prelink
  glBindAttribLocation(program, attr_pos, "pos");
  glBindAttribLocation(program, attr_uv, "uv");

  linkShaderProgram(program);

  // has to happen postlink
  u_resolution   = glGetUniformLocation(program, "u_resolution");
  u_texture      = glGetUniformLocation(program, "s_texture");
  u_mask         = glGetUniformLocation(program, "s_mask");
  u_enablemask   = glGetUniformLocation(program, "u_enablemask");
//  u_flipmaskcoords = glGetUniformLocation(program, "u_flipmaskcoords");
//  u_enablepremultipliedalpha = glGetUniformLocation(program, "u_enablepremultipliedalpha");
  u_matrix       = glGetUniformLocation(program, "amymatrix");
  u_alpha        = glGetUniformLocation(program, "u_alpha");
  u_color        = glGetUniformLocation(program, "a_color");
  u_alphatexture = glGetUniformLocation(program, "u_alphatexture");

  glEnable(GL_BLEND);

  // assume non-premultiplied for now... 
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
// non-premultiplied
//  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//  glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(program);

  gprogram = program;
}

void pxContext::setSize(int w, int h)
{
  glViewport(0, 0, (GLint)w, (GLint)h);
//  glUniform2f(u_resolution, w, h);
  gResW = w;
  gResH = h;
  if (currentRenderSurface == defaultRenderSurface)
  {
    defaultContextSurface.width = w;
    defaultContextSurface.height = h;
  }
}

void pxContext::clear(int /*w*/, int /*h*/)
{
  glClear(GL_COLOR_BUFFER_BIT);
}

void pxContext::setMatrix(pxMatrix4f& m)
{
//  glUseProgram(gprogram);
//  glUniformMatrix4fv(u_matrix, 1, GL_FALSE, m.data());
  gMatrix = m;
}

void pxContext::setAlpha(float a)
{
//  glUseProgram(gprogram);
//  glUniform1f(u_alpha, a); 
  gAlpha = a;
}

pxTextureRef pxContext::createContextSurface(int width, int height)
{
  pxFBOTexture* texture = new pxFBOTexture();
  texture->createTexture(width, height);
  
  return texture;
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
  if (texture.getPtr() == NULL)
  {
    glUseProgram(gprogram);
    glViewport ( 0, 0, defaultContextSurface.width, defaultContextSurface.height);
//    glUniform2f(u_resolution, defaultContextSurface.width, defaultContextSurface.height);
    gResW = defaultContextSurface.width;
    gResH = defaultContextSurface.height;
#if 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId1);
#endif
    // TODO probably need to save off the original FBO handle rather than assuming zero
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    currentRenderSurface = defaultRenderSurface;
    return PX_OK;
  }
  
  currentRenderSurface = texture;
  return texture->prepareForRendering();
}

pxError deleteContextSurface(pxTextureRef texture)
{
  if (texture.getPtr() == NULL)
  {
    return PX_FAIL;
  }
  return texture->deleteTexture();
}

void pxContext::drawRect(float w, float h, float lineWidth, float* fillColor, float* lineColor)
{
  float half = lineWidth/2;
  drawRect2(half, half, w-lineWidth, h-lineWidth, fillColor);
  if (lineWidth > 0)
    drawRectOutline(0, 0, w, h, lineWidth, lineColor);
}


void pxContext::drawImage9(float w, float h, float x1, float y1,
                           float x2, float y2, pxTextureRef texture)
{
  if (texture.getPtr() != NULL) {
    drawImage92(0, 0, w, h, x1, y1, x2, y2, texture);
  }
}

void pxContext::drawImage(float x, float y, float w, float h, pxTextureRef t, pxTextureRef mask,
                          pxStretch xStretch, pxStretch yStretch, float* color) 
{
  float black[4] = {0,0,0,1};
  drawImageTexture(x, y, w, h, t, mask, xStretch, yStretch, color?color:black);
}

void pxContext::drawDiagRect(float x, float y, float w, float h, float* color)
{
  if (!mShowOutlines) return;

  const float verts[4][2] =
  {
    { x,y },
    {  x+w, y },
    {  x+w, y+h },
    {  x,  y+h },
   };
  

  float colorPM[4];
  memcpy(&colorPM, color, sizeof(colorPM));
  colorPM[0] *= colorPM[3];
  colorPM[1] *= colorPM[3];
  colorPM[2] *= colorPM[3];
  
  gSolidShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_LINE_LOOP,verts,4,colorPM); 
}

void pxContext::drawDiagLine(float x1, float y1, float x2, float y2, float* color)
{
  if (!mShowOutlines) return;
  const float verts[4][2] =
  {
    { x1, y1 },
    { x2, y2 },
   };
  
  float colorPM[4];
  memcpy(&colorPM, color, sizeof(colorPM));
  colorPM[0] *= colorPM[3];
  colorPM[1] *= colorPM[3];
  colorPM[2] *= colorPM[3];
  
  gSolidShader->draw(gResW,gResH,gMatrix.data(),gAlpha,GL_LINES,verts,2,colorPM); 
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
