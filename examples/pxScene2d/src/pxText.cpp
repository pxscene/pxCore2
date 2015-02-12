// pxCore CopyRight 2007-2015 John Robinson
// pxText.cpp

#include "pxText.h"

#include <math.h>
#include <map>

struct GlyphCacheEntry
{
  int bitmap_left;
  int bitmap_top;
  int bitmapdotwidth;
  int bitmapdotrows;
  //void* bitmapdotbuffer;
  int advancedotx;
  int advancedoty;
  int vertAdvance;

  pxTextureRef mTexture;
};
typedef map<pair<uint32_t,uint32_t>, GlyphCacheEntry*> GlyphCache;

GlyphCache gGlyphCache;

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


uint32_t gSize = 0;
void ftSetSize(uint32_t s)
{
  if (gSize != s)
  {
    FT_Set_Pixel_Sizes(face, 0, s);
    gSize = s;
  }
}

const GlyphCacheEntry* getGlyph(uint32_t codePoint)
{
  GlyphCache::iterator it = gGlyphCache.find(make_pair(gSize,codePoint));
  if (it != gGlyphCache.end())
    return it->second;
  else
  {
    if(FT_Load_Char(face, codePoint, FT_LOAD_RENDER))
      return NULL;
    else
    {
      printf("glyph cache miss\n");
      GlyphCacheEntry *entry = new GlyphCacheEntry;
      FT_GlyphSlot g = face->glyph;
      
      entry->bitmap_left = g->bitmap_left;
      entry->bitmap_top = g->bitmap_top;
      entry->bitmapdotwidth = g->bitmap.width;
      entry->bitmapdotrows = g->bitmap.rows;
      entry->advancedotx = g->advance.x;
      entry->advancedoty = g->advance.y;
      entry->vertAdvance = g->metrics.vertAdvance;

      entry->mTexture = context.createTexture(g->bitmap.width, g->bitmap.rows, 
                                              g->bitmap.width, g->bitmap.rows, 
                                              g->bitmap.buffer);
      
      gGlyphCache.insert(pair<pair<uint32_t,uint32_t>, GlyphCacheEntry*>(make_pair(gSize,codePoint), entry));
      return entry;
    }
  }
}

void measureText(const char* text, uint32_t size,  float sx, float sy, float& w, float& h) {

  ftSetSize(size);

  w = 0; h = 0;
  if (!text) return;
  int i = 0;
  u_int32_t codePoint;

  FT_Size_Metrics* metrics = &face->size->metrics;
  h = metrics->height>>6;

  float lw = 0;
  while((codePoint = u8_nextchar((char*)text, &i)) != 0) {

    const GlyphCacheEntry* entry = getGlyph(codePoint);
    if (!entry) continue;
 
    if (codePoint != '\n')
    {
      lw += (entry->advancedotx >> 6) * sx;
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

  ftSetSize(size);
  FT_Size_Metrics* metrics = &face->size->metrics;

  while((codePoint = u8_nextchar((char*)text, &i)) != 0) {

    const GlyphCacheEntry* entry = getGlyph(codePoint);
    if (!entry) continue;

    float x2 = x + entry->bitmap_left * sx;
//    float y2 = y - g->bitmap_top * sy;
    float y2 = (y - entry->bitmap_top * sy) + (metrics->ascender>>6);
    float w = entry->bitmapdotwidth * sx;
    float h = entry->bitmapdotrows * sy;

    if (codePoint != '\n')
    {
      if (x == 0) {
        float c[4] = {0, 1, 0, 1};
        context.drawDiagLine(0, y+(metrics->ascender>>6), mw, 
                             y+(metrics->ascender>>6), c);
      }

      pxTextureRef texture = entry->mTexture;
      pxTextureRef nullImage;
      context.drawImage(x2,y2, w, h, texture, nullImage, PX_NONE, PX_NONE, color);
      x += (entry->advancedotx >> 6) * sx;
      // TODO not sure if this is right?  seems weird commenting out to see what happens
      y += (entry->advancedoty >> 6) * sy;
    }
    else
    {
      x = 0;
      // TODO not sure if this is right?
      y += entry->vertAdvance>>6;
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
