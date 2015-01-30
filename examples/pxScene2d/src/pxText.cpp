// pxCore CopyRight 2007-2015 John Robinson
// pxText.cpp

#include "pxText.h"

#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "pxContext.h"

extern pxContext context;

extern "C" {
#include "utf8.h"
}

#if 0
<link href='http://fonts.googleapis.com/css?family=Fontdiner+Swanky' rel='stylesheet' type='text/css'>
#endif

FT_Library ft;
FT_Face face;

void initFT() 
{
  static bool init = false;
  if (init) return;
  init = true;
  
  if(FT_Init_FreeType(&ft)) {
    fprintf(stderr, "Could not init freetype library\n");
    return;
  }
  
//  if(FT_New_Face(ft, "FreeSans.ttf", 0, &face)) {
  if(FT_New_Face(ft, "FontdinerSwanky.ttf", 0, &face)) {
    rtLogError("Could not load font face: \n");
    return;
  }
  
  FT_Set_Pixel_Sizes(face, 0, 64);
}

void measureText(const char* text, float sx, float sy, float& w, float& h) {
  w = 0; h = 0;
  if (!text) return;
  int i = 0;
  u_int32_t codePoint;

  while((codePoint = u8_nextchar((char*)text, &i)) != 0) {

    // TODO don't render glyph
    if(FT_Load_Char(face, codePoint, FT_LOAD_RENDER)) {
      rtLog("Could not load glyph: %d\n", codePoint);
      continue;
    }
    
    FT_GlyphSlot g = face->glyph;

    w += (g->advance.x >> 6) * sx;
    h = pxMax<float>((g->advance.y >> 6) * sy, h);
  }

}

void renderText(const char *text, float x, float y, float sx, float sy, float* color) {
  if (!text) return;
  int i = 0;
  u_int32_t codePoint;

  //printf("renderText %s\n", text);
  while((codePoint = u8_nextchar((char*)text, &i)) != 0) {

    if(FT_Load_Char(face, codePoint, FT_LOAD_RENDER)) {
      rtLogError("Could not load glyph: %d\n", codePoint);
      continue;
    }
    
    FT_GlyphSlot g = face->glyph;

    float x2 = x + g->bitmap_left * sx;
    float y2 = y - g->bitmap_top * sy;
    float w = g->bitmap.width * sx;
    float h = g->bitmap.rows * sy;

    if (codePoint != '\n')
    {
      context.drawImageAlpha(x2, y2, w, h, g->bitmap.width, g->bitmap.rows, g->bitmap.buffer, color);
      x += (g->advance.x >> 6) * sx;
      // TODO not sure if this is right?  seems weird commenting out to see what happens
      y += (g->advance.y >> 6) * sy;
    }
    else
    {
      x = 0;
      // TODO not sure if this is right?
      y += g->metrics.vertAdvance>>6;
    }
  }
}

pxText::pxText() {
  initFT();
  float c[4] = {0, 0, 0, 1};
  memcpy(mTextColor, c, sizeof(mTextColor));
}

rtError pxText::text(rtString& s) const { s = mText; return RT_OK; }

rtError pxText::setText(const char* s) { 
  mText = s; 
  measureText(s, 1.0, 1.0, mw, mh);
  return RT_OK; 
}

void pxText::draw() {
  renderText(mText, 0, 0, 1.0, 1.0, mTextColor);
}

rtDefineObject(pxText, pxObject);
rtDefineProperty(pxText, text);
rtDefineProperty(pxText, textColor);

