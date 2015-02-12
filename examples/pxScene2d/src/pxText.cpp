// pxCore CopyRight 2007-2015 John Robinson
// pxText.cpp

#include "pxText.h"

#include <math.h>
#include <map>

struct GlyphKey {
  uint32_t mFaceId;
  uint32_t mPixelSize;
  uint32_t mCodePoint;

  // Provide a "<" operator that orders keys.
  // The way it orders them doesn't matter, all that matters is that
  // it orders them consistently.
  bool operator<(GlyphKey const& other) const {
    if (mFaceId < other.mFaceId) return true; else
      if (mFaceId == other.mFaceId) {
        if (mPixelSize < other.mPixelSize) return true; else
          if (mPixelSize == other.mPixelSize) {
            if (mCodePoint < other.mCodePoint) return true;
          }
      }
    return false;
  }
};

typedef map<GlyphKey,GlyphCacheEntry*> GlyphCache;

GlyphCache gGlyphCache;

#include "pxContext.h"

extern pxContext context;

extern "C" {
#include "utf8.h"
}

#if 0
<link href='http://fonts.googleapis.com/css?family=Fontdiner+Swanky' rel='stylesheet' type='text/css'>
#endif

FT_Library ft;


// Weak Map
typedef map<rtString, pxFace*> FaceMap;
FaceMap gFaceMap;

uint32_t gFaceId = 0;

pxFace::pxFace() { mFaceId = gFaceId++; }

rtError pxFace::init(const char* n)
{
  if(FT_New_Face(ft, n, 0, &mFace))
    return RT_FAIL;
  
  mFaceName = n;
  setPixelSize(defaultPixelSize);
  gFaceMap.insert(make_pair(n, this));
  return RT_OK;
}

pxFace::~pxFace() 
{ 
  rtLogInfo("~pxFace"); 
  FaceMap::iterator it = gFaceMap.find(mFaceName);
  if (it != gFaceMap.end())
    gFaceMap.erase(it);
  else
    rtLogError("Could not find faceName in map");
}

void pxFace::setPixelSize(uint32_t s)
{
  if (mPixelSize != s)
  {
    FT_Set_Pixel_Sizes(mFace, 0, s);
    mPixelSize = s;
  }
}
  
const GlyphCacheEntry* pxFace::getGlyph(uint32_t codePoint)
{
  GlyphKey key; 
  key.mFaceId = mFaceId; 
  key.mPixelSize = mPixelSize; 
  key.mCodePoint = codePoint;
  GlyphCache::iterator it = gGlyphCache.find(key);
  if (it != gGlyphCache.end())
    return it->second;
  else
  {
    if(FT_Load_Char(mFace, codePoint, FT_LOAD_RENDER))
      return NULL;
    else
    {
      rtLogInfo("glyph cache miss");
      GlyphCacheEntry *entry = new GlyphCacheEntry;
      FT_GlyphSlot g = mFace->glyph;
      
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
      
#if 0
      GlyphKey key;
      key.mFaceId = mFaceId;
      key.mPixelSize = mPixelSize;
      key.mCodePoint = codePoint;
#endif
      gGlyphCache.insert(make_pair(key,entry));
      return entry;
    }
  }
  return NULL;
}
  
void pxFace::measureText(const char* text, uint32_t size,  float sx, float sy, 
                         float& w, float& h) 
{
  
  setPixelSize(size);
  
  w = 0; h = 0;
  if (!text) 
    return;
  int i = 0;
  u_int32_t codePoint;
  
  FT_Size_Metrics* metrics = &mFace->size->metrics;
  
  h = metrics->height>>6;
  float lw = 0;
  while((codePoint = u8_nextchar((char*)text, &i)) != 0) 
  {
    const GlyphCacheEntry* entry = getGlyph(codePoint);
    if (!entry) 
      continue;
      
    if (codePoint != '\n')
    {
      lw += (entry->advancedotx >> 6) * sx;
    }
    else
    {
      //h += metrics->height>>6;
      h += (metrics->height)>>6;
      lw = 0;
    }
    w = pxMax<float>(w, lw);
//    h = pxMax<float>((g->advance.y >> 6) * sy, h);
//    h = pxMax<float>((metrics->height >> 6) * sy, h);
  }
  h *= sy;
}

void pxFace::renderText(const char *text, uint32_t size, float x, float y, 
                        float sx, float sy, 
                        float* color, float mw) 
{
  if (!text) 
    return;

  int i = 0;
  u_int32_t codePoint;

  setPixelSize(size);
  FT_Size_Metrics* metrics = &mFace->size->metrics;

  while((codePoint = u8_nextchar((char*)text, &i)) != 0) 
  {
    const GlyphCacheEntry* entry = getGlyph(codePoint);
    if (!entry) 
      continue;

    float x2 = x + entry->bitmap_left * sx;
//    float y2 = y - g->bitmap_top * sy;
    float y2 = (y - entry->bitmap_top * sy) + (metrics->ascender>>6);
    float w = entry->bitmapdotwidth * sx;
    float h = entry->bitmapdotrows * sy;
    
    if (codePoint != '\n')
    {
      if (x == 0) 
      {
        float c[4] = {0, 1, 0, 1};
        context.drawDiagLine(0, y+(metrics->ascender>>6), mw, 
                             y+(metrics->ascender>>6), c);
      }
      
      pxTextureRef texture = entry->mTexture;
      pxTextureRef nullImage;
      context.drawImage(x2,y2, w, h, texture, nullImage, PX_NONE, PX_NONE, 
                        color);
      x += (entry->advancedotx >> 6) * sx;
      // TODO not sure if this is right?  seems weird commenting out to see what happens
      y += (entry->advancedoty >> 6) * sy;
    }
    else
    {
      x = 0;
      // TODO not sure if this is right?
      y += (entry->vertAdvance>>6) * sy;
    }
  }
}

typedef rtRefT<pxFace> pxFaceRef;

pxFaceRef gFace;

void initFT() 
{
  static bool init = false;

  if (init) 
    return;

  init = true;
  
  if(FT_Init_FreeType(&ft)) 
  {
    rtLogError("Could not init freetype library\n");
    return;
  }
  
  pxFaceRef f = new pxFace;
  if (f->init(defaultFace) != RT_OK)
    rtLogError("Could not load default face, %s\n", defaultFace);
  else
    gFace = f;

}

pxText::pxText() {
  initFT();
  float c[4] = {1, 1, 1, 1};
  memcpy(mTextColor, c, sizeof(mTextColor));
  mPixelSize = defaultPixelSize;
  mFace = gFace;
}

rtError pxText::text(rtString& s) const { s = mText; return RT_OK; }

rtError pxText::setText(const char* s) { 
  mText = s; 
  mFace->measureText(s, mPixelSize, 1.0, 1.0, mw, mh);
  return RT_OK; 
}

rtError pxText::setPixelSize(uint32_t v) 
{   
  mPixelSize = v; 
  mFace->measureText(mText, mPixelSize, 1.0, 1.0, mw, mh);
  return RT_OK; 
}

void pxText::draw() {
  mFace->renderText(mText, mPixelSize, 0, 0, 1.0, 1.0, mTextColor, mw);
}

rtError pxText::setFaceURL(const char* s)
{
  if (!s || !s[0])
    s = defaultFace;

  FaceMap::iterator it = gFaceMap.find(s);
  if (it != gFaceMap.end())
    mFace = it->second;
  else
  {
    pxFaceRef f = new pxFace;
    rtError e = f->init(s);
    if (e != RT_OK)
    {
      rtLogInfo("Could not load font face, %s, %s\n", "blah", s);
      return e;
    }
    else
      mFace = f;
  }
  mFaceURL = s;
  
  return RT_OK;
}

rtDefineObject(pxText, pxObject);
rtDefineProperty(pxText, text);
rtDefineProperty(pxText, textColor);
rtDefineProperty(pxText, pixelSize);
rtDefineProperty(pxText, faceURL);
