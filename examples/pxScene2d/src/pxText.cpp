// pxCore CopyRight 2007-2015 John Robinson
// pxText.cpp

#include "pxText.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library ft;
FT_Face face;
GLuint vbo;

extern GLint u_texture;
extern GLint u_alphatexture;
extern GLint attr_pos, attr_uv;


void initFT() {
  static bool init = false;
  if (init) return;
  init = true;

  if(FT_Init_FreeType(&ft)) {
    fprintf(stderr, "Could not init freetype library\n");
    return;
  }
  
  if(FT_New_Face(ft, "FreeSans.ttf", 0, &face)) {
    fprintf(stderr, "Could not open font\n");
    return;
  }
  
  FT_Set_Pixel_Sizes(face, 0, 128);
}

void renderText(const char *text, float x, float y, float sx, float sy) {
  if (!text) return;

  const char *p;

  for(p = text; *p; p++) {

    if(FT_Load_Char(face, *p, FT_LOAD_RENDER)) {
      printf("danger will robinson\n");
      continue;
    }
    
    FT_GlyphSlot g = face->glyph;
    
    glActiveTexture(GL_TEXTURE1);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_ALPHA,
      g->bitmap.width,
      g->bitmap.rows,
      0,
      GL_ALPHA,
      GL_UNSIGNED_BYTE,
      g->bitmap.buffer
    );
    
    glUniform1i(u_texture, 1);
    glUniform1f(u_alphatexture, 2.0);
    
    float x2 = x + g->bitmap_left * sx;
    float y2 = y - g->bitmap_top * sy;
    float w = g->bitmap.width * sx;
    float h = g->bitmap.rows * sy;

    const GLfloat verts[4][2] = {
      { x2,y2 },
      {  x2+w, y2 },
      {  x2,  y2+h },
      {  x2+w, y2+h }
    };
    
    const GLfloat uv[4][2] = {
      { 0, 0 },
      { 1, 0 },
      { 0, 1 },
      { 1, 1 }
    };
    
    {
      glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
      glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
      glEnableVertexAttribArray(attr_pos);
      glEnableVertexAttribArray(attr_uv);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glDisableVertexAttribArray(attr_pos);
      glDisableVertexAttribArray(attr_uv);
    }

    x += (g->advance.x >> 6) * sx;
    y += (g->advance.y >> 6) * sy;
  }
}

pxText::pxText() {
  initFT();
}

void pxText::text(rtString& s) { s = mText; }

void pxText::setText(const char* s) { mText = s; }

void pxText::draw() {
  renderText(mText, 0, 0, 1.0, 1.0);
}

