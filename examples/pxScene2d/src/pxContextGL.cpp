#include "rtDefs.h"
#include "rtLog.h"
#include "pxContext.h"


#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#ifdef PX_PLATFORM_WAYLAND_EGL
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
GLint u_alphatexture = -1;
GLint u_color = -1;
GLint attr_pos = 0, attr_uv = 2;

static const char *fShaderText =
#ifdef PX_PLATFORM_WAYLAND_EGL
  "precision mediump float;\n"
#endif
  "uniform float u_alphatexture;\n"
  "uniform float u_alpha;\n"
  "uniform vec4 a_color;\n"
  "uniform sampler2D s_texture;\n"
  "varying vec2 v_uv;\n"
  "void main() {\n"
  "if (u_alphatexture < 1.0) {"
  // solid color
  " gl_FragColor = a_color;"
  "} else "
  "if (u_alphatexture < 2.0) {\n"
  // image
  "gl_FragColor = texture2D(s_texture, v_uv);\n"
  "} else {\n"
  // text
  "gl_FragColor = vec4(a_color.r, a_color.g, a_color.b, texture2D(s_texture, v_uv).a*a_color.a);"
  "}\n"
  "gl_FragColor.a *= u_alpha;"
  "}\n";

static const char *vShaderText =
  "uniform vec2 u_resolution;\n"
  "uniform mat4 amymatrix;\n"
  "attribute vec2 pos;\n"
  "attribute vec2 uv;\n"
  "varying vec2 v_uv;\n"
  "void main() {\n"
  // map to "pixel coordinates"
  " vec4 p = amymatrix * vec4(pos, 0, 1);\n"
  " vec4 zeroToOne = p / vec4(u_resolution, u_resolution.x, 1);\n"
  " vec4 zeroToTwo = zeroToOne * vec4(2.0, 2.0, 1, 1);\n"
  " vec4 clipSpace = zeroToTwo - vec4(1.0, 1.0, 0, 0);\n"
  " clipSpace.w = 1.0+clipSpace.z;\n"
  " gl_Position =  clipSpace * vec4(1, -1, 1, 1);\n"
  "v_uv = uv;\n"
  "}\n";


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
    rtLogError("Error: fragment shader did not compile: ", glGetError());
    
    GLint maxLength = 0;
    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);
    
    //The maxLength includes the NULL character
    std::vector<char> errorLog(maxLength);
    glGetShaderInfoLog(fragShader, maxLength, &maxLength, &errorLog[0]);
    
    //Provide the infolog in whatever manor you deem best.
    rtLog("%s\n", &errorLog[0]);
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


static void draw9SliceRect(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
  float ox1 = x;
  float ix1 = x+x1;
  float ox2 = x+w;
  float ix2 = x+w-x2;
  float oy1 = y;
  float iy1 = y+y1;
  float oy2 = y+h;
  float iy2 = y+h-y2;
  
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
#if 0
  const GLfloat colors[4][3] =
  {
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 0, 1, 0 },
    { 0, 0, 1 }
  };
  const GLfloat uv[22][2] =
  {
    { 0, 0 },
    { 1, 0 },
    { 0, 1 },
    { 1, 1 }
  };
#endif
  
  
  {
    glUniform1f(u_alphatexture, 0.0);
    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(attr_pos);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 22);
    glDisableVertexAttribArray(attr_pos);
  }
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


static void drawSurface2(float x, float y, float w, float h, pxContextSurfaceNativeDesc* contextSurface)
{
  if ((contextSurface == NULL) || (contextSurface->texture == 0))
  {
    return;
  }
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, contextSurface->texture);
  glUniform1i(u_texture, 0);

  const float verts[4][2] =
  {
    { x,y },
    {  x+w, y },
    {  x,  y+h },
    {  x+w, y+h }
  };

  const float uv[4][2] =
  {
    { 0, 0 },
    { 1, 0 },
    { 0, 1 },
    { 1, 1 }
  };
  
  {
    glUniform1f(u_alphatexture, 1.0);
    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_uv);
  }
  
  glBindTexture(GL_TEXTURE_2D, textureId1); //bind back to original texture
}


static void drawImage2(float x, float y, float w, float h, pxOffscreen& offscreen)
{
  glActiveTexture(GL_TEXTURE0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
	       offscreen.width(), offscreen.height(), 0, GL_BGRA_EXT,
	       GL_UNSIGNED_BYTE, offscreen.base());

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(u_texture, 0);

  const float verts[4][2] =
  {
    { x,y },
    {  x+w, y },
    {  x,  y+h },
    {  x+w, y+h }
  };

  const float uv[4][2] =
  {
    { 0, 0 },
    { 1, 0 },
    { 0, 1 },
    { 1, 1 }
  };
  
  {
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


static void drawImage92(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
    pxOffscreen& offscreen)
{
  glActiveTexture(GL_TEXTURE0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
	       offscreen.width(), offscreen.height(), 0, GL_BGRA_EXT,
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
#if !defined(__APPLE__) && !defined(PX_PLATFORM_WAYLAND_EGL)

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

  glClearColor(0.4, 0.4, 0.4, 0.0);

  GLuint program = createShaderProgram(vShaderText, fShaderText);

  u_resolution   = glGetUniformLocation(program, "u_resolution");
  u_texture      = glGetUniformLocation(program, "s_texture");
  u_matrix       = glGetUniformLocation(program, "amymatrix");
  u_alpha        = glGetUniformLocation(program, "u_alpha");
  u_color        = glGetUniformLocation(program, "a_color");
  u_alphatexture = glGetUniformLocation(program, "u_alphatexture");

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &textureId1);
  glBindTexture(GL_TEXTURE_2D, textureId1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
}


void pxContext::clear(int w, int h)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glUniform2f(u_resolution, w, h);
}


void pxContext::setMatrix(pxMatrix4f& m)
{
  glUniformMatrix4fv(u_matrix, 1, GL_FALSE, m.data());
}


void pxContext::setAlpha(float a)
{
  glUniform1f(u_alpha, a); 
}


pxError pxContext::createContextSurface(pxContextSurfaceNativeDesc* contextSurface, int width, int height)
{
  if (contextSurface == NULL)
  {
    rtLog("cannot create context surface because contextSurface is NULL");
    return PX_FAIL;
  }
  deleteContextSurface(contextSurface);
  contextSurface->width = width;
  contextSurface->height = height;
  
  glGenFramebuffers(1, &contextSurface->framebuffer);
  glGenTextures(1, &contextSurface->texture);
  glBindTexture(GL_TEXTURE_2D, contextSurface->texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
	       width, height, 0, GL_BGRA_EXT,
	       GL_UNSIGNED_BYTE, NULL);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  return PX_OK;
}


pxError pxContext::setRenderSurface(pxContextSurfaceNativeDesc* contextSurface)
{
  if (contextSurface == NULL)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return PX_OK;
  }
  
  if ((contextSurface->framebuffer == 0) || (contextSurface->texture == 0))
  {
    rtLog("render surface is not initialized\n");
    return PX_NOTINITIALIZED;
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, contextSurface->framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                         GL_TEXTURE_2D, contextSurface->texture, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    rtLog("error setting the render surface\n");
    return PX_FAIL;
  }
  
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  
  return PX_OK;
}


pxError pxContext::deleteContextSurface(pxContextSurfaceNativeDesc* contextSurface)
{
  if (contextSurface == NULL)
  {
    return PX_OK;
  }

  if (contextSurface->framebuffer != 0)
  {
    glDeleteFramebuffers(1, &contextSurface->framebuffer);
    contextSurface->framebuffer = 0;
  }

  if (contextSurface->texture != 0)
  {
    glDeleteTextures(1, &contextSurface->texture);
    contextSurface->texture = 0;
  }

  return PX_OK;
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


void pxContext::drawImage9(float w, float h, pxOffscreen& o)
{
  drawImage92(0, 0, w, h, 75,75,75,75, o);
}


void pxContext::drawImage(float w, float h, pxOffscreen& o)
{
  drawImage2(0, 0, w, h, o);
}


void pxContext::drawSurface(float w, float h, pxContextSurfaceNativeDesc* contextSurface)
{
  drawSurface2(0, 0, w, h, contextSurface);
}


void pxContext::drawImageAlpha(float x, float y, float w, float h, int bw, int bh, void* buffer, float* color)
{
    glActiveTexture(GL_TEXTURE1);
    
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

