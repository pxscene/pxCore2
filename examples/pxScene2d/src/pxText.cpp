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

#define defaultFontSize 64

void initFT() 
{
  static bool init = false;

  if (init) 
    return;

  init = true;
  
  if(FT_Init_FreeType(&ft)) 
  {
    fprintf(stderr, "Could not init freetype library\n");
    return;
  }
  
  if(FT_New_Face(ft, "FreeSans.ttf", 0, &face)) 
  //if(FT_New_Face(ft, "FontdinerSwanky.ttf", 0, &face))
  {
    rtLogError("Could not load font face: ");
    return;
  }
  
  FT_Set_Pixel_Sizes(face, 0, defaultFontSize);
}

void measureText(const char* text, uint32_t size,  float sx, float sy, float& w, float& h) {

  FT_Set_Pixel_Sizes(face, 0, size);

  w = 0; h = 0;
  if (!text) return;
  int i = 0;
  u_int32_t codePoint;

  FT_Size_Metrics* metrics = &face->size->metrics;
  h = metrics->height>>6;

  float lw = 0;
  while((codePoint = u8_nextchar((char*)text, &i)) != 0) {

    // TODO don't render glyph
    if(FT_Load_Char(face, codePoint, FT_LOAD_RENDER)) {
      rtLogWarn("Could not load glyph: %d", codePoint);
      continue;
    }
    
    FT_GlyphSlot g = face->glyph;
 
    if (codePoint != '\n')
    {
      lw += (g->advance.x >> 6) * sx;
    }
    else
    {
      h += metrics->height>>6;
      lw = 0;
    }
    w = pxMax<float>(w, lw);
//    h = pxMax<float>((g->advance.y >> 6) * sy, h);
//    h = pxMax<float>((metrics->height >> 6) * sy, h);
  }
  h *= sy;
}

void renderText(const char *text, uint32_t size, float x, float y, float sx, float sy, 
                float* color, float mw) {
  if (!text) return;
  int i = 0;
  u_int32_t codePoint;

  FT_Set_Pixel_Sizes(face, 0, size);
  FT_Size_Metrics* metrics = &face->size->metrics;

  while((codePoint = u8_nextchar((char*)text, &i)) != 0) {

    if(FT_Load_Char(face, codePoint, FT_LOAD_RENDER)) {
      rtLogError("Could not load glyph: %d", codePoint);
      continue;
    }
    
    FT_GlyphSlot g = face->glyph;

    float x2 = x + g->bitmap_left * sx;
//    float y2 = y - g->bitmap_top * sy;
    float y2 = (y - g->bitmap_top * sy) + (metrics->ascender>>6);
    float w = g->bitmap.width * sx;
    float h = g->bitmap.rows * sy;

    if (codePoint != '\n')
    {
      if (x == 0) {
        float c[4] = {0, 1, 0, 1};
        context.drawDiagLine(0, y+(metrics->ascender>>6), mw, 
                             y+(metrics->ascender>>6), c);
      }
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
  float c[4] = {1, 1, 1, 1};
  memcpy(mTextColor, c, sizeof(mTextColor));
  mPixelSize = 64;
}

rtError pxText::text(rtString& s) const { s = mText; return RT_OK; }

rtError pxText::setText(const char* s) { 
  mText = s; 
  measureText(s, mPixelSize, 1.0, 1.0, mw, mh);
  return RT_OK; 
}

rtError pxText::setPixelSize(uint32_t v) 
{   
  mPixelSize = v; 
  return RT_OK; 
}

void pxText::draw() {
  renderText(mText, mPixelSize, 0, 0, 1.0, 1.0, mTextColor, mw);
}

rtDefineObject(pxText, pxObject);
rtDefineProperty(pxText, text);
rtDefineProperty(pxText, textColor);
rtDefineProperty(pxText, pixelSize);
