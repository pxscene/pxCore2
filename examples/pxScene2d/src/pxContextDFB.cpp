#include "rtDefs.h"
#include "rtLog.h"
#include "pxContext.h"

#include <directfb.h>

/* Place holder for a dfb implementation of pxContext */


extern IDirectFB        *dfb;
extern IDirectFBSurface *primary;
extern IDirectFBWindow  *window;

#define DFBCHECK(x...)              \
{                                   \
  DFBResult err = x;                \
                                    \
  if (err != DFB_OK)                \
{                                   \
  rtLogError( " DFB died... " );    \
  DirectFBErrorFatal( #x, err );    \
  }                                 \
}



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
  if(primary == 0)
  {
    rtLog("cannot drawRect2 on context surface because surface is NULL");
    return;
  }

  DFBCHECK (primary->FillRectangle (primary, x, y, w, h));
}


static void drawRectOutline(float x, float y, float w, float h, float lw)
{
  if(primary == 0)
  {
    rtLog("cannot drawRect2 on context surface because surface is NULL");
    return;
  }

  DFBCHECK (primary->DrawRectangle (primary, x, y, w, h));


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
  if ((contextSurface == NULL) || (contextSurface->texture == 0))
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
  if(primary == 0)
  {
    rtLog("cannot clear on context surface because surface is NULL");
    return;
  }

  DFBCHECK (primary->FillRectangle (primary, 0, 0, w, h));

//Clear the screen by filling a rectangle with the size of the surface.
//Default color is black, default drawing flags are DSDRAW_NOFX.
}


void pxContext::setMatrix(pxMatrix4f& m)
{
//  glUniformMatrix4fv(u_matrix, 1, GL_FALSE, m.data());
}


void pxContext::setAlpha(float a)
{
//  glUniform1f(u_alpha, a);
  window->SetOpacity(window, a);
}


pxError pxContext::createContextSurface(pxContextSurfaceNativeDesc* contextSurface, int width, int height)
{
  if (dfb == NULL)
  {
    rtLog("cannot create context surface because DFB is NULL");
    return PX_FAIL;
  }

  if (contextSurface == NULL)
  {
    rtLog("cannot create context surface because contextSurface is NULL");
    return PX_FAIL;
  }

  deleteContextSurface(contextSurface);

  contextSurface->dsc.width  = width;
  contextSurface->dsc.height = height;

  contextSurface->width  = width;
  contextSurface->height = height;

  DFBCHECK (dfb->CreateSurface( dfb, &contextSurface->dsc, &contextSurface->surface ));

  return PX_OK;
}


pxError pxContext::setRenderSurface(pxContextSurfaceNativeDesc* contextSurface)
{
  if(contextSurface == 0)
  {
    rtLog("cannot setRenderSurface on context surface because surface is NULL");
    return PX_FAIL;
  }

  primary = contextSurface->surface;

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


pxError pxContext::deleteContextSurface(pxContextSurfaceNativeDesc* contextSurface)
{
  if (contextSurface == NULL)
  {
    return PX_OK;
  }

  IDirectFBSurface *surface = contextSurface->surface; //alias

  if(surface != 0)
  {
     surface->Release(surface);
  }

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
  if(primary == 0)
  {
    rtLog("cannot drawRect on context surface because surface is NULL");
    return;
  }

  DFBCHECK (primary->SetColor(primary, fillColor[0] * 255.0,
                                       fillColor[1] * 255.0,
                                       fillColor[2] * 255.0,
                                       fillColor[3] * 255.0));
  float half = lineWidth/2;

  drawRect2(half, half, w-lineWidth, h-lineWidth);

  if (lineWidth > 0)
  {
    DFBCHECK (primary->SetColor(primary, lineColor[0] * 255.0,
                                         lineColor[1] * 255.0,
                                         lineColor[2] * 255.0,
                                         lineColor[3] * 255.0));

    drawRectOutline(0, 0, w, h, lineWidth);
  }
}


void pxContext::drawImage9(float w, float h, pxOffscreen& o)
{
  drawImage92(0, 0, w, h, 75,75,75,75, o);
}


void pxContext::drawImage(float w, float h, pxOffscreen& o,
                          pxStretch xStretch, pxStretch yStretch)
{
  drawImage2(0, 0, w, h, o, xStretch, yStretch);
}


void pxContext::drawSurface(float w, float h, pxContextSurfaceNativeDesc* contextSurface)
{
  drawSurface2(0, 0, w, h, contextSurface);
}


void pxContext::drawImageAlpha(float x, float y, float w, float h, int bw, int bh, void* buffer, float* color)
{
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


