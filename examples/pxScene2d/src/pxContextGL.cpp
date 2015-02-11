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

GLuint textureId1, textureId2;

GLint attribute_coord;

pxContextSurfaceNativeDesc defaultContextSurface;
pxContextSurfaceNativeDesc* currentContextSurface = &defaultContextSurface;

pxTextureRef defaultRenderSurface;
pxTextureRef currentRenderSurface = defaultRenderSurface;

struct point
{
    GLfloat x;
    GLfloat y;
    GLfloat s;
    GLfloat t;
};

static GLint u_matrix = -1;
static GLint u_alpha = -1;
static GLint u_resolution = -1;
GLint u_texture = -1;
GLint u_mask = -1;
GLint u_enablemask = 0;
GLint u_alphatexture = -1;
GLint u_color = -1;
GLint attr_pos = 0, attr_uv = 2;

static const char *fShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;\n"
#endif
  "uniform float u_alphatexture;\n"
  "uniform float u_alpha;\n"
  "uniform vec4 a_color;\n"
  "uniform sampler2D s_texture;\n"
  "uniform sampler2D s_mask;\n"
  "uniform int u_enablemask;\n"
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
  "    textureColor.a = textureColor.a * maskColor.a;\n" ////textureColor.a * maskColor.a;
  "  }\n"
  "  gl_FragColor = textureColor;\n"
  "} else {\n"
  // text
  "gl_FragColor = vec4(a_color.r, a_color.g, a_color.b, texture2D(s_texture, v_uv).a*a_color.a);"
  "}\n"
  "gl_FragColor.a *= u_alpha;"
#else
  "gl_FragColor = vec4(1,1,1,1);"
#endif
  "}\n";

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


class pxTextureGL : public pxTexture
{
public:
  pxTextureGL() : mWidth(0), mHeight(0), mTextureId(0)
  {
    mTextureType = PX_TEXTURE_NATIVE;
  }

  ~pxTextureGL() { deleteTexture(); }

  void createTexture(int width, int height)
  {
    if (mTextureId != 0)
    {
      deleteTexture();
    }
    glGenTextures(1, &mTextureId);
    mWidth = width;
    mHeight = height;
  }
  
  virtual pxError deleteTexture()
  {
    if (mTextureId != 0)
    {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
    }
    return PX_OK;
  }

  virtual pxError bindTexture()
  {
    if (mTextureId == 0)
    {
      return PX_NOTINITIALIZED;
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextureId);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(u_texture, 0);
    return PX_OK;
  }
  
  virtual pxError getOffscreen(pxOffscreen& o)
  {
    (void)o;
    // TODO
    return PX_FAIL;
  }

  virtual float width() { return mWidth; }
  virtual float height() { return mHeight; }

private:
  GLfloat mWidth;
  GLfloat mHeight;
  GLuint mTextureId;
};

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
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    glUniform2f(u_resolution, mWidth, mHeight);
    
    return PX_OK;
  }

  virtual pxError bindTexture()
  {
    if (mFramebufferId == 0 || mTextureId == 0)
    {
      return PX_NOTINITIALIZED;
    }

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mTextureId);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glUniform1i(u_texture, 3);
    
    return PX_OK;
  }
  
  virtual pxError getOffscreen(pxOffscreen& o)
  {
    (void)o;
    // TODO
    return PX_FAIL;
  }

  virtual float width() { return mWidth; }
  virtual float height() { return mHeight; }

private:
  GLfloat mWidth;
  GLfloat mHeight;
  GLuint mFramebufferId;
  GLuint mTextureId;
};

int getTextureUnit()
{
  static int t = 4;

  return t++;
}

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
    createTexture(o);
  }

  ~pxTextureOffscreen() { deleteTexture(); };
  
  void createTexture(pxOffscreen& o)
  {
    mOffscreen.init(o.width(), o.height());
    o.blit(mOffscreen);
    mInitialized = true;
#if 0
    mTextureUnit = getTextureUnit();
    printf("===\n");
    printf("max texture units %d\n", GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    int texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    printf("num texture units%d\n", texture_units);
    printf("GL_TEXTURE0: %d\n", GL_TEXTURE0);
    printf("texture unit%d\n", mTextureUnit);
    glActiveTexture(GL_TEXTURE0+mTextureUnit);
    glGenTextures(1, &mTextureName);
    printf("texture name%d\n", mTextureName);
    glBindTexture(GL_TEXTURE_2D, mTextureName);
#if 1
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
           mOffscreen.width(), mOffscreen.height(), 0, GL_RGBA,
           GL_UNSIGNED_BYTE, mOffscreen.base());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#endif
  }
  
  virtual pxError deleteTexture()
  {
    mInitialized = false;
    return PX_OK;
  }

  virtual pxError bindTexture()
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }
    
#if 1
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                 mOffscreen.width(), mOffscreen.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, mOffscreen.base());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glUniform1i(u_texture, 0);
#else
    glActiveTexture(mTextureUnit); // should we try to get rid of texture state changes during drawing
    glUniform1i(u_texture, mTextureUnit);
#endif
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

  virtual float width() { return mOffscreen.width(); }
  virtual float height() { return mOffscreen.height(); }
  
private:
  pxOffscreen mOffscreen;
  bool mInitialized;
  GLuint mTextureName;
  int mTextureUnit;
};

class pxTextureMask : public pxTexture
{
public:
  pxTextureMask() : mOffscreen(), mInitialized(false)
  {
    mTextureType = PX_TEXTURE_ALPHA;
  }

  pxTextureMask(pxOffscreen& o) : mOffscreen(), mInitialized(false)
  {
    mTextureType = PX_TEXTURE_ALPHA;
    createTexture(o);
  }
  
  pxTextureMask(pxTextureRef texture) : mOffscreen(), mInitialized(false)
  {
    mTextureType = PX_TEXTURE_ALPHA;
    if (texture.getPtr() != NULL && texture->getOffscreen(mOffscreen) == PX_OK)
    {
      mInitialized = true;
    }
  }

  ~pxTextureMask() { deleteTexture(); };
  
  void createTexture(pxOffscreen& o)
  {
    mOffscreen.init(o.width(), o.height());
    o.blit(mOffscreen);
    mInitialized = true;
  }
  
  virtual pxError deleteTexture()
  {
    mInitialized = false;
    return PX_OK;
  }

  virtual pxError bindTexture()
  {
    if (!mInitialized)
    {
      return PX_NOTINITIALIZED;
    }
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureId1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                 mOffscreen.width(), mOffscreen.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, mOffscreen.base());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glUniform1i(u_mask, 2);
    glUniform1i(u_enablemask, 1);
    
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

  virtual float width() { return mOffscreen.width(); }
  virtual float height() { return mOffscreen.height(); }
  
private:
  pxOffscreen mOffscreen;
  bool mInitialized;
};

GLuint createShaderProgram(const char* vShaderTxt, const char* fShaderTxt)
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
  glLinkProgram(program);
  
  glGetProgramiv(program, GL_LINK_STATUS, &stat);
  if (!stat)
  {
    char log[1000];
    GLsizei len;
    glGetProgramInfoLog(program, 1000, &len, log);
    rtLogError("faild to link:%s", log);
    exit(1);
  }
  
  /* test setting attrib locations */
  glBindAttribLocation(program, attr_pos, "pos");
  glBindAttribLocation(program, attr_uv, "uv");
  glLinkProgram(program);  /* needed to put attribs into effect */
  
  return program;
}

static void drawRect2(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
  const float verts[4][2] =
  {
    { x,y },
    {  x+w, y },
    {  x,  y+h },
    {  x+w, y+h }
  };
  
  {
    glUniform1f(u_alphatexture, 0.0);
    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(attr_pos);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(attr_pos);
  }
}


static void drawRectOutline(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat lw)
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
  
  {
    glUniform1f(u_alphatexture, 0.0);

    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(attr_pos);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
    glDisableVertexAttribArray(attr_pos);
  }
}

static void drawImage2(float x, float y, float w, float h, pxOffscreen& offscreen,
                pxStretch xStretch, pxStretch yStretch)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureId1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
	       offscreen.width(), offscreen.height(), 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, offscreen.base());

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(u_texture, 0);

  float iw = offscreen.width();
  float ih = offscreen.height();

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

  const float uv[4][2] = {
    { 0,  0 },
    { tw, 0 },
    { 0,  th },
    { tw, th }
  };
  
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glUniform1f(u_alphatexture, 1.0);
    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_uv);
  }
}

static void drawImageTexture(float x, float y, float w, float h, pxTextureRef texture,
                pxTextureRef mask, pxStretch xStretch, pxStretch yStretch)
{
  if (texture.getPtr() == NULL)
  {
    return;
  }
#if 1
  texture->bindTexture();
#endif
  
  if (mask.getPtr() != NULL && mask->getType() == PX_TEXTURE_ALPHA)
  {
    mask->bindTexture();
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
  
  float firstTextureY = 0;
  float secondTextureY = th;
  
  if (texture->getType() == PX_TEXTURE_FRAME_BUFFER)
  {
    //opengl renders to a framebuffer in reverse y coordinates than pxConext renders.  
    //the texture y values need to be flipped
    firstTextureY = th;
    secondTextureY = 0;
  }

  const float uv[4][2] = {
    { 0,  firstTextureY },
    { tw, firstTextureY },
    { 0,  secondTextureY },
    { tw, secondTextureY }
  };

  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glUniform1f(u_alphatexture, 1.0);
    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_uv);
  }
  
  glUniform1i(u_enablemask, 0);
}

static void drawImage92(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat x1, GLfloat y1, GLfloat x2, 
                        GLfloat y2, pxOffscreen& offscreen)
{
  glActiveTexture(GL_TEXTURE0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
	       offscreen.width(), offscreen.height(), 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, offscreen.base());

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(u_texture, 0);

  float ox1 = x;
  float ix1 = x+x1;
  float ix2 = x+w-x2;
  float ox2 = x+w;

  float oy1 = y;
  float iy1 = y+y1;
  float iy2 = y+h-y2;
  float oy2 = y+h;


  
  float w2 = offscreen.width();
  float h2 = offscreen.height();

  float ou1 = 0;
  float iu1 = x1/w2;
  float iu2 = (w2-x2)/w2;
  float ou2 = 1;

  float ov1 = 0;
  float iv1 = y1/h2;
  float iv2 = (h2-y2)/h2;
  float ov2 = 1;

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
  iv1 = tmin;
  iv2 = tmax;

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

  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1f(u_alphatexture, 1.0);
    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_uv);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 22);

    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_uv);
  }
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

  GLuint program = createShaderProgram(vShaderText, fShaderText);

  u_resolution   = glGetUniformLocation(program, "u_resolution");
  u_texture      = glGetUniformLocation(program, "s_texture");
  u_mask         = glGetUniformLocation(program, "s_mask");
  u_enablemask   = glGetUniformLocation(program, "u_enablemask");
  u_matrix       = glGetUniformLocation(program, "amymatrix");
  u_alpha        = glGetUniformLocation(program, "u_alpha");
  u_color        = glGetUniformLocation(program, "a_color");
  u_alphatexture = glGetUniformLocation(program, "u_alphatexture");

  // Using for RGBA texture
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &textureId1);
  glBindTexture(GL_TEXTURE_2D, textureId1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if 0
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif

  // Using for alpha only texture
  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1, &textureId2);
  glBindTexture(GL_TEXTURE_2D, textureId2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  glEnable(GL_BLEND);

  // assume non-premultiplied for now... 
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

  glUseProgram(program);
}


void pxContext::setSize(int w, int h)
{
  glViewport(0, 0, (GLint)w, (GLint)h);
  glUniform2f(u_resolution, w, h);
  if (currentRenderSurface == defaultRenderSurface)
  {
    defaultContextSurface.width = w;
    defaultContextSurface.height = h;
  }
}


void pxContext::clear(int w, int h)
{
  glClear(GL_COLOR_BUFFER_BIT);
}


void pxContext::setMatrix(pxMatrix4f& m)
{
  glUniformMatrix4fv(u_matrix, 1, GL_FALSE, m.data());
}


void pxContext::setAlpha(float a)
{
  glUniform1f(u_alpha, a); 
}

pxTextureRef pxContext::createContextSurface(int width, int height)
{
  pxFBOTexture* texture = new pxFBOTexture();
  texture->createTexture(width, height);
  
  return texture;
}

pxTextureRef pxContext::getCurrentRenderSurface()
{
  return currentRenderSurface;
}

pxError pxContext::setRenderSurface(pxTextureRef texture)
{
  if (texture.getPtr() == NULL)
  {
    glViewport ( 0, 0, defaultContextSurface.width, defaultContextSurface.height);
    glUniform2f(u_resolution, defaultContextSurface.width, defaultContextSurface.height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId1);
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
  glUniform4fv(u_color, 1, fillColor);
  float half = lineWidth/2;

  drawRect2(half, half, w-lineWidth, h-lineWidth);

  if (lineWidth > 0)
  {
    glUniform4fv(u_color, 1, lineColor);
    drawRectOutline(0, 0, w, h, lineWidth);
  }
}


void pxContext::drawImage9(float w, float h, float x1, float y1,
                           float x2, float y2, pxOffscreen& o)
{
  drawImage92(0, 0, w, h, x1, y1, x2, y2, o);
}

void pxContext::drawImage(float w, float h, pxOffscreen& o,
                          pxStretch xStretch, pxStretch yStretch) 
{
  drawImage2(0, 0, w, h, o, xStretch, yStretch);
}

void pxContext::drawImage(float w, float h, pxTextureRef t, pxTextureRef mask,
                          pxStretch xStretch, pxStretch yStretch) 
{
  drawImageTexture(0, 0, w, h, t, mask, xStretch, yStretch);
}

void pxContext::drawImageAlpha(float x, float y, float w, float h, int bw, int bh, void* buffer, float* color)
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureId2);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_ALPHA,
      bw,
      bh,
      0,
      GL_ALPHA,
      GL_UNSIGNED_BYTE,
      buffer
    );
    
    glUniform1i(u_texture, 1);
    glUniform1f(u_alphatexture, 2.0);
    
    const GLfloat verts[4][2] =
    {
      { x,y },
      {  x+w, y },
      {  x,  y+h },
      {  x+w, y+h }
    };
    
    const GLfloat uv[4][2] =
    {
      { 0, 0 },
      { 1, 0 },
      { 0, 1 },
      { 1, 1 }
    };
    
    {
      glUniform4fv(u_color, 1, color);
      glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
      glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
      glEnableVertexAttribArray(attr_pos);
      glEnableVertexAttribArray(attr_uv);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      glDisableVertexAttribArray(attr_pos);
      glDisableVertexAttribArray(attr_uv);
    }  
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
  
  {
    glUniform4fv(u_color, 1, color);
    glUniform1f(u_alphatexture, 0.0);
    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(attr_pos);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDisableVertexAttribArray(attr_pos);
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
    glUniform4fv(u_color, 1, color);
    glUniform1f(u_alphatexture, 0.0);
    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(attr_pos);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableVertexAttribArray(attr_pos);
  }
}

pxTextureRef pxContext::createTexture(pxOffscreen& o)
{
  pxTextureOffscreen* offscreenTexture = new pxTextureOffscreen(o);
  return offscreenTexture;
}

pxTextureRef pxContext::createMask(pxTextureRef t)
{
  pxTextureMask* maskTexture = new pxTextureMask(t);
  return maskTexture;
}
