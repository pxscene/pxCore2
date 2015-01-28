// pxCore CopyRight 2007-2015 John Robinson
// pxText.cpp

#include "pxText.h"

#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "pxContext.h"

extern pxContext context;


#if 0
<link href='http://fonts.googleapis.com/css?family=Fontdiner+Swanky' rel='stylesheet' type='text/css'>
#endif

FT_Library ft;
FT_Face face;

void initFT() {
  static bool init = false;
  if (init) return;
  init = true;

  if(FT_Init_FreeType(&ft)) {
    fprintf(stderr, "Could not init freetype library\n");
    return;
  }
  
  //  if(FT_New_Face(ft, "FreeSans.ttf", 0, &face)) {
  if(FT_New_Face(ft, "FontdinerSwanky.ttf", 0, &face)) {
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

    float x2 = x + g->bitmap_left * sx;
    float y2 = y - g->bitmap_top * sy;
    float w = g->bitmap.width * sx;
    float h = g->bitmap.rows * sy;


    
#if 0
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
#else
    context.renderGlyph(x2, y2, w, h, g->bitmap.width, g->bitmap.rows, g->bitmap.buffer);
#endif
    x += (g->advance.x >> 6) * sx;
    y += (g->advance.y >> 6) * sy;
  }
}

pxText::pxText() {
  initFT();
}

rtError pxText::text(rtString& s) const { s = mText; return RT_OK; }

rtError pxText::setText(const char* s) { mText = s; return RT_OK; }

void pxText::draw() {
  renderText(mText, 0, 0, 1.0, 1.0);
}

rtDefineObject(pxText, pxObject);
rtDefineProperty(pxText, text);


