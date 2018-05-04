/*

 pxCore Copyright 2005-2018 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// pxFont.cpp

#include "rtFileDownloader.h"
#include "pxFont.h"
#include "pxTimer.h"
#include "pxText.h"

#include <math.h>
#include <map>


#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cctype>

#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_BBOX_H

using namespace std;

struct GlyphKey 
{
  uint32_t mFontId;
  uint32_t mPixelSize;
  uint32_t mCodePoint;

  // Provide a "<" operator that orders keys.
  // The way it orders them doesn't matter, all that matters is that
  // it orders them consistently.
  bool operator<(GlyphKey const& other) const {
    if (mFontId < other.mFontId) return true; else
      if (mFontId == other.mFontId) {
        if (mPixelSize < other.mPixelSize) return true; else
          if (mPixelSize == other.mPixelSize) {
            if (mCodePoint < other.mCodePoint) return true;
          }
      }
    return false;
  }
};

typedef map<GlyphKey,GlyphCacheEntry*> GlyphCache;
typedef map<GlyphKey,GlyphTextureEntry> GlyphTextureCache;

GlyphCache gGlyphCache;
GlyphTextureCache gGlyphTextureCache;

#include "pxContext.h"

extern pxContext context;

#if 1
// TODO can we eliminate direct utf8.h usage
extern "C" {
#include "../../../src/utf8.h"
}
#endif

FT_Library ft;
uint32_t gFontId = 1;

// TODO move out to rt* utility
uint32_t npot(uint32_t i)
{
  uint32_t power = 1;
  while(power < i)
    power*=2;
  return power;
}

#ifdef PXSCENE_FONT_ATLAS
pxFontAtlas gFontAtlas;
#endif

pxFont::pxFont(rtString fontUrl, uint32_t id, rtString proxyUrl):pxResource(),mFace(NULL),mPixelSize(0), mFontData(0), mFontDataSize(0),
             mFontMutex(), mFontDataMutex(), mFontDownloadedData(NULL), mFontDownloadedDataSize(0), mFontDataUrl()
{  
  mFontId = id; 
  mUrl = fontUrl;
  mProxy = proxyUrl;
}

pxFont::~pxFont() 
{
  rtLogInfo("~pxFont %s\n", mUrl.cString());
  if (gUIThreadQueue)
  {
    gUIThreadQueue->removeAllTasksForObject(this);
  }
  // download should be canceled/removed in pxResource
  //if (mDownloadRequest != NULL)
  //{
    //// clear any pending downloads
    //mFontDownloadRequest->setCallbackFunctionThreadSafe(NULL);
  //}  
   
  pxFontManager::removeFont( mFontId);
 
  if( mInitialized) 
  {
    FT_Done_Face(mFace);
  }
  mFace = 0;
  
  if(mFontData) {
    free(mFontData);
    mFontData = 0;
    mFontDataSize = 0;
  }

  clearDownloadedData();
}

void pxFont::setFontData(const FT_Byte*  fontData, FT_Long size, const char* n)
{
  mFontDataMutex.lock();
  mFontDataUrl = n;
  if (mFontDownloadedData != NULL)
  {
    delete [] mFontDownloadedData;
    mFontDownloadedData = NULL;
    mFontDownloadedDataSize = 0;
  }
  if (fontData == NULL)
  {
    mFontDownloadedData = NULL;
    mFontDownloadedDataSize = 0;
  }
  else
  {
    mFontDownloadedData = new char[size];
    mFontDownloadedDataSize = size;
    memcpy(mFontDownloadedData, fontData, mFontDownloadedDataSize);
  }
  mFontDataMutex.unlock();
}

void pxFont::clearDownloadedData()
{
  mFontDataMutex.lock();
  if (mFontDownloadedData != NULL)
  {
    delete [] mFontDownloadedData;
    mFontDownloadedData = NULL;
  }
  mFontDownloadedDataSize = 0;
  mFontDataMutex.unlock();
}

void pxFont::setupResource()
{
  if (!mInitialized)
  {
    mFontDataMutex.lock();
    if (mFontDownloadedData != NULL)
    {
      init( (FT_Byte*)mFontDownloadedData,
            (FT_Long)mFontDownloadedDataSize,
            mFontDataUrl.cString());
      delete [] mFontDownloadedData;
      mFontDownloadedData = NULL;
      mFontDownloadedDataSize = 0;
    }
    mFontDataMutex.unlock();
  }
}

bool pxFont::loadResourceData(rtFileDownloadRequest* fileDownloadRequest)
{
      // Load the font data
    setFontData( (FT_Byte*)fileDownloadRequest->downloadedData(),
            (FT_Long)fileDownloadRequest->downloadedDataSize(),
            fileDownloadRequest->fileUrl().cString());
            
      return true;
}

void pxFont::loadResourceFromFile()
{
    rtError e = init(mUrl);
    if (e != RT_OK)
    {
      rtLogWarn("Could not load font face %s\n", mUrl.cString());
      setLoadStatus("statusCode", PX_RESOURCE_STATUS_FILE_NOT_FOUND);
      // Since this object can be released before we get a async completion
      // We need to maintain this object's lifetime
      // TODO review overall flow and organization
      AddRef();
      if (gUIThreadQueue)
      {
        gUIThreadQueue->addTask(onDownloadCompleteUI, this, (void*)"reject");
      }
    }
    else
    {
      setLoadStatus("statusCode", PX_RESOURCE_STATUS_OK);
      // Since this object can be released before we get a async completion
      // We need to maintain this object's lifetime
      // TODO review overall flow and organization
      AddRef();
      if (gUIThreadQueue)
      {
        gUIThreadQueue->addTask(onDownloadCompleteUI, this, (void*)"resolve");
      }

    } 
}

// This init(char*) is for load of local font files
rtError pxFont::init(const char* n)
{
  mFontMutex.lock();
  mUrl = n;
   
  if(FT_New_Face(ft, n, 0, &mFace)) {
    mFontMutex.unlock();
    return RT_FAIL;
  }
  
  mInitialized = true;
  setPixelSize(defaultPixelSize);
  mFontMutex.unlock();
  return RT_OK;
}
// This init is used by async callback to load downloaded font file data
rtError pxFont::init(const FT_Byte*  fontData, FT_Long size, const char* n)
{
  mFontMutex.lock();
  // We need to keep a copy of fontData since the download will be deleted.
  mFontData = (char *)malloc(size);
  memcpy(mFontData, fontData, size);
  mFontDataSize = size;
  mUrl = n;
  if(FT_New_Memory_Face(ft, (const FT_Byte*)mFontData, mFontDataSize, 0, &mFace))
  {
    mFontMutex.unlock();
    return RT_FAIL;
  }

  mInitialized = true;
  setPixelSize(defaultPixelSize);

  mFontMutex.unlock();
  
  return RT_OK;
}

void pxFont::setPixelSize(uint32_t s)
{
  if (mPixelSize != s && mInitialized)
  {
    //rtLogDebug("pxFont::setPixelSize size=%d mPixelSize=%d mInitialized=%d and mFace=%d\n", s,mPixelSize,mInitialized, mFace);
    FT_Set_Pixel_Sizes(mFace, 0, s);
    mPixelSize = s;
  }
}
void pxFont::getHeight(uint32_t size, float& height)
{
	// TO DO:  check FT_IS_SCALABLE 
  if( !mInitialized) 
  {
    rtLogWarn("getHeight called on font before it is initialized\n");
    return;
  }
  
  setPixelSize(size);
  
	FT_Size_Metrics* metrics = &mFace->size->metrics;
  	
	height = metrics->height>>6; 
}
  
void pxFont::getMetrics(uint32_t size, float& height, float& ascender, float& descender, float& naturalLeading)
{
	// TO DO:  check FT_IS_SCALABLE 
  if( !mInitialized) 
  {
    rtLogWarn("Font getMetrics called on font before it is initialized\n");
    return;
  }
  if(!size) {
    rtLogWarn("Font getMetrics called with pixelSize=0\n");
  }
  
  setPixelSize(size);
  
	FT_Size_Metrics* metrics = &mFace->size->metrics;
  	
	height = metrics->height>>6;
	ascender = metrics->ascender>>6; 
	descender = -metrics->descender>>6; 
  naturalLeading = height - (ascender + descender);

}

GlyphTextureEntry pxFont::getGlyphTexture(uint32_t codePoint, float sx, float sy)
{
  GlyphTextureEntry result;
  // Select a glyph texture better suited for rendering the glyph
  // taking pixelSize and scale into account
  uint32_t pixelSize=(uint32_t)ceil((sx>sy?sx:sy)*mPixelSize);
  
  //  TODO:  FIXME: Disabled for now.   Sub-Pixel rounding making some Glyphs too "wide" at certain sizes.
  //
#if 0
  if (pixelSize < 8)
  {
    pixelSize = (pixelSize + 7) & 0xfffffff8;    // next multiple of 8
  }
  else if (pixelSize <= 32)
  {
    //pixelSize = (pixelSize + 7) & 0xfffffff8;  // next multiple of 8
    pixelSize += (pixelSize % 2);
  }
  else
    pixelSize = npot(pixelSize);  // else next power of two
#else
  pixelSize = mPixelSize; // HACK
#endif
  
  
  GlyphKey key; 
  key.mFontId = mFontId; 
  key.mPixelSize = pixelSize; 
  key.mCodePoint = codePoint;
  GlyphTextureCache::iterator it = gGlyphTextureCache.find(key);
  if (it != gGlyphTextureCache.end())
    return it->second;
  else
  {
    // temporarily set pixel size to more optimal size for
    // rendering texture 
    FT_Set_Pixel_Sizes(mFace, 0, pixelSize);
    // TODO only need to render glyph here
    if(!FT_Load_Char(mFace, codePoint, FT_LOAD_RENDER))
    {
      rtLogDebug("glyph texture cache miss");

      FT_GlyphSlot g = mFace->glyph;

#ifdef PXSCENE_FONT_ATLAS
      if (!gFontAtlas.addGlyph(g->bitmap.width, g->bitmap.rows, g->bitmap.buffer, result))
      {
#endif
        rtLogWarn("Glyph not in atlas");
        result.t = context.createTexture(g->bitmap.width, g->bitmap.rows, 
                                                g->bitmap.width, g->bitmap.rows, 
                                                g->bitmap.buffer);

        result.u1 = 0;
        result.v1 = 1;
        result.u2 = 1;
        result.v2 = 0;
#ifdef PXSCENE_FONT_ATLAS
      }
#endif
      
      gGlyphTextureCache.insert(make_pair(key,result));

      // restore current pixelSize
      FT_Set_Pixel_Sizes(mFace, 0, mPixelSize);
      return result;  
    }
    // restore current pixelSize
    FT_Set_Pixel_Sizes(mFace, 0, mPixelSize);
  }
  return result;  
}
  
const GlyphCacheEntry* pxFont::getGlyph(uint32_t codePoint)
{
  GlyphKey key; 
  key.mFontId = mFontId; 
  key.mPixelSize = mPixelSize; 
  key.mCodePoint = codePoint;
  GlyphCache::iterator it = gGlyphCache.find(key);
  if (it != gGlyphCache.end())
    return it->second;
  else
  {
    // TODO should not need to render here !
    if(FT_Load_Char(mFace, codePoint, FT_LOAD_RENDER))
      return NULL;
    else
    {
      rtLogDebug("glyph cache miss");
      GlyphCacheEntry *entry = new GlyphCacheEntry;
      FT_GlyphSlot g = mFace->glyph;
      
      entry->bitmap_left = g->bitmap_left;
      entry->bitmap_top = g->bitmap_top;
      entry->bitmapdotwidth = g->bitmap.width;
      entry->bitmapdotrows = g->bitmap.rows;
      entry->advancedotx = (int32_t) g->advance.x;
      entry->advancedoty = (int32_t) g->advance.y;
      entry->vertAdvance = (int32_t) g->metrics.vertAdvance; // !CLF: Why vertAdvance? SHould only be valid for vert layout of text.

      gGlyphCache.insert(make_pair(key,entry));

      return entry;
    }
  }
  return NULL;
}

void pxFont::measureTextInternal(const char* text, uint32_t size,  float sx, float sy, 
                         float& w, float& h) 
{
  w = 0; h = 0;

  // TODO ignoring sx and sy now
  sx = 1.0;
  sy = 1.0;
  if( !mInitialized) 
  {
    rtLogWarn("measureText called TOO EARLY -- not initialized or font not loaded!\n");
    return;
  }

  setPixelSize(size);

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
      h += (metrics->height>>6) *sy;
      lw = 0;
    }
    w = pxMax<float>(w, lw);
//    h = pxMax<float>((g->advance.y >> 6) * sy, h);
//    h = pxMax<float>((metrics->height >> 6) * sy, h);
  }
  h *= sy;
}

void pxFont::renderText(const char *text, uint32_t size, float x, float y, 
                        float nsx, float nsy, 
                        float* color, float mw) 
{
  if (!text || !mInitialized)
  { 
    rtLogWarn("renderText called on font before it is initialized\n");
    return;
  }

  int i = 0;
  u_int32_t codePoint;

  setPixelSize(size);
  FT_Size_Metrics* metrics = &mFace->size->metrics;
  
  while((codePoint = u8_nextchar((char*)text, &i)) != 0) 
  {
    const GlyphCacheEntry* entry = getGlyph(codePoint);
    if (!entry) 
      continue;

    float x2 = x + entry->bitmap_left;
    //float y2 = y - g->bitmap_top;
    float y2 = (y - entry->bitmap_top) + (metrics->ascender>>6);
    float w = entry->bitmapdotwidth;
    float h = entry->bitmapdotrows;
    
    if (codePoint != '\n')
    {
      if (x == 0) 
      {
        float c[4] = {0, 1, 0, 1};
        context.drawDiagLine(0, y+(metrics->ascender>>6), mw, 
                             y+(metrics->ascender>>6), c);
      }
      
      GlyphTextureEntry texture = getGlyphTexture(codePoint, nsx, nsy);

      pxTextureRef nullImage;

      #ifdef PXSCENE_FONT_ATLAS
      const float verts[6][2] =
      {
        { x2,     y2 },
        { x2+w,   y2 },
        { x2,   y2+h },
        { x2+w,   y2 },
        { x2,   y2+h },
        { x2+w, y2+h }
      };
      const float uvs[6][2] =
      {
        { texture.u1,  texture.v1  },
        { texture.u2, texture.v1  },
        { texture.u1,  texture.v2 },
        { texture.u2, texture.v1  },
        { texture.u1,  texture.v2 },
        { texture.u2, texture.v2 }
      };
      context.drawTexturedQuads(1, verts, uvs, texture.t, color);
      #else
      context.drawImage(x2,y2, w, h, texture.t, nullImage, false, color);
      #endif
      
      x += (entry->advancedotx >> 6);
      // no change to y because we are not moving to next line yet
    }
    else
    {
      x = 0;
      // Use height to advance to next line
      y += (metrics->height>>6);
    }
  }
}

#ifdef PXSCENE_FONT_ATLAS
void pxFont::renderTextToQuads(const char *text, uint32_t size, 
                        float nsx, float nsy, 
                        pxTexturedQuads& quads, 
                        float x, float y) 
{
  quads.clear();
  if (!text || !mInitialized)
  { 
    rtLogWarn("renderText called on font before it is initialized\n");
    return;
  }

  int i = 0;
  u_int32_t codePoint;

  setPixelSize(size);
  FT_Size_Metrics* metrics = &mFace->size->metrics;
  
  while((codePoint = u8_nextchar((char*)text, &i)) != 0) 
  {
    GlyphCacheEntry* entry = (GlyphCacheEntry*)getGlyph(codePoint);

    if (!entry) 
      continue;

    float x2 = x + entry->bitmap_left;
//    float y2 = y - g->bitmap_top;
    float y2 = (y - entry->bitmap_top) + (metrics->ascender>>6);
    float w = entry->bitmapdotwidth;
    float h = entry->bitmapdotrows;
    
    if (codePoint != '\n')
    {
      
      GlyphTextureEntry t = getGlyphTexture(codePoint, nsx, nsy);

      pxTextureRef nullImage;

      quads.addQuad(x2,y2,x2+w,y2+h,t.u1,t.v1,t.u2,t.v2,t.t);

      x += (entry->advancedotx >> 6);
      // no change to y because we are not moving to next line yet
    }
    else
    {
      x = 0;
      // Use height to advance to next line
      y += (metrics->height>>6);
    }
  }
}
#endif

void pxFont::measureTextChar(u_int32_t codePoint, uint32_t size,  float sx, float sy, 
                         float& w, float& h) 
{
  // TODO ignoring sx and sy now
  sx = 1.0;
  sy = 1.0;
  if( !mInitialized) {
    rtLogWarn("measureTextChar called TOO EARLY -- not initialized or font not loaded!\n");
    return;
  }
    
  setPixelSize(size);
  
  w = 0; h = 0;
  
  FT_Size_Metrics* metrics = &mFace->size->metrics;
  
  h = metrics->height>>6;
  float lw = 0;

  const GlyphCacheEntry* entry = getGlyph(codePoint);
  if (!entry) 
    return;

  lw = (entry->advancedotx >> 6) * sx;

  w = pxMax<float>(w, lw);

  h *= sy;
}


/*
#### getFontMetrics - returns information about the font (font and size).  It does not convey information about the text of the font.  
* See section 3.a in http://www.freetype.org/freetype2/docs/tutorial/step2.html .  
* The returned object has the following properties:
* height - float - the distance between baselines
* ascent - float - the distance from the baseline to the font ascender (note that this is a hint, not a solid rule)
* descent - float - the distance from the baseline to the font descender  (note that this is a hint, not a solid rule)
*/
rtError pxFont::getFontMetrics(uint32_t pixelSize, rtObjectRef& o) 
{
  //rtLogDebug("pxFont::getFontMetrics\n");  
	float height, ascent, descent, naturalLeading;
	pxTextMetrics* metrics = new pxTextMetrics();

  if(!mInitialized ) 
  {
    rtLogWarn("getFontMetrics called TOO EARLY -- not initialized or font not loaded!\n");
    o = metrics;
    return RT_OK; // !CLF: TO DO - COULD RETURN RT_ERROR HERE TO CATCH NOT WAITING ON PROMISE
  }

	getMetrics(pixelSize, height, ascent, descent, naturalLeading);
	metrics->setHeight(height);
	metrics->setAscent(ascent);
	metrics->setDescent(descent);
  metrics->setNaturalLeading(naturalLeading);
  metrics->setBaseline(ascent);
	o = metrics;

	return RT_OK;
}

/** Public API exposed to java script **/
rtError pxFont::measureText(uint32_t pixelSize, rtString stringToMeasure, rtObjectRef& o)
{
  pxTextSimpleMeasurements* measure = new pxTextSimpleMeasurements();
  
  if(!mInitialized ) 
  {
    rtLogWarn("measureText called TOO EARLY -- not initialized or font not loaded!\n");
    o = measure;
    return RT_OK; // !CLF: TO DO - COULD RETURN RT_ERROR HERE TO CATCH NOT WAITING ON PROMISE
  } 
  
  if(!pixelSize) {
    rtLogWarn("Font measureText called with pixelSize=0\n");
  }    
  
  float w, h;
  measureTextInternal(stringToMeasure, pixelSize, 1.0,1.0, w, h);
  //rtLogDebug("pxFont::measureText returned %f and %f for pixelSize=%d and text \"%s\"\n",w, h,pixelSize, stringToMeasure.cString());
  measure->setW(w);
  measure->setH(h);
  o = measure;
  
  return RT_OK; 
}


/**********************************************************************/
/**                    pxFontManager                                 **/
/**********************************************************************/
FontMap pxFontManager::mFontMap;
FontIdMap pxFontManager::mFontIdMap;
bool pxFontManager::init = false;
void pxFontManager::initFT() 
{
  if (init) 
  {
    //rtLogDebug("initFT returning; already inited\n");
    return;
  }
  init = true;
  
  if(FT_Init_FreeType(&ft)) 
  {
    rtLogError("Could not init freetype library\n");
    return;
  }
  
}
rtRef<pxFont> pxFontManager::getFont(const char* url, const char* proxy)
{
  initFT();

  rtRef<pxFont> pFont;
  uint32_t fontId;

  if (!url || !url[0])
    url = defaultFont;

  // Assign font urls an id number if they don't have one
   FontIdMap::iterator itId = mFontIdMap.find(url);
  if( itId != mFontIdMap.end()) 
  {
    fontId = itId->second;
  }
  else 
  {
    fontId = gFontId++;
    mFontIdMap.insert(make_pair(url, fontId));
  }

  FontMap::iterator it = mFontMap.find(fontId);
  if (it != mFontMap.end())
  {
    rtLogDebug("Found pxFont in map for %s\n",url);
    pFont = it->second;
    return pFont;  
    
  }
  else 
  {
    rtLogDebug("Create pxFont in map for %s\n",url);
    pFont = new pxFont(url, fontId, proxy);
    mFontMap.insert(make_pair(fontId, pFont));
    pFont->loadResource();
  }
  
  return pFont;
}

void pxFontManager::removeFont(uint32_t fontId)
{
  FontMap::iterator it = mFontMap.find(fontId);
  if (it != mFontMap.end())
  {  
    mFontMap.erase(it);
  }
}

void pxFontManager::clearAllFonts()
{
  for (GlyphCache::iterator it =  gGlyphCache.begin(); it != gGlyphCache.end(); it++)
    delete it->second;

  gGlyphCache.clear();
  gGlyphTextureCache.clear();
  mFontIdMap.clear();
#ifdef PXSCENE_FONT_ATLAS
  gFontAtlas.clearTexture();
#endif
}


/**********************************************************************/
/**                    OutlinePrinter                                **/
/**********************************************************************/

class OutlinePrinter
{
public:
  OutlinePrinter(FT_Face &face);
  int GlyphToSvg(const char *symbol);
 
  rtString getPath();
  rtString getRawPath();

private:
  void LoadGlyph(const char *symbol) const;
  bool OutlineExists() const;
  void FlipOutline() const;
  void ExtractOutline();
  void ComputeViewBox();
  void PrintSVG() const;
  
  static int moveTo(  const FT_Vector *to,  void *user);
  static int lineTo(  const FT_Vector *to,  void *user);
  static int conicTo( const FT_Vector *pt, const FT_Vector *to, void *user);
  static int cubicTo( const FT_Vector *pt1, const FT_Vector *pt2, const FT_Vector *to, void *user);
  
private:
  
  FT_Face &m_Face;
  
  ostringstream m_path;
  
  // These four variables are for the `viewBox' attribute.
  FT_Pos m_xMin;
  FT_Pos m_yMin;
  FT_Pos m_width;
  FT_Pos m_height;
};


inline
OutlinePrinter::OutlinePrinter(FT_Face &face)
    : m_Face(face), m_xMin(0), m_yMin(0), m_width(0), m_height(0)
{
  // Empty body.
}


int
OutlinePrinter::GlyphToSvg(const char *symbol)
{
  LoadGlyph(symbol);
  
  // Check whether outline exists.
  bool outlineExists = OutlineExists();
  
  if (!outlineExists) // Outline doesn't exist.
    return -1;
//    throw runtime_error("Outline check failed.\n"
//                        "Please, inspect your font file or try another one,"
//                        " for example LiberationSerif-Bold.ttf");
  
  FlipOutline();
  
  ExtractOutline();
  
  ComputeViewBox();
  
 // PrintSVG();
  
  return 0;
}

void
OutlinePrinter::LoadGlyph(const char *symbol) const
{
  FT_ULong code = symbol[0];
  
  // For simplicity, use the charmap FreeType provides by default;
  // in most cases this means Unicode.
  FT_UInt index = FT_Get_Char_Index(m_Face, code);
  
  FT_Error error = FT_Load_Glyph(m_Face,
                                 index,
                                 FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP);
  
  if (error)
    throw runtime_error("Couldn't load the glyph: FT_Load_Glyph() failed");
}


// While working on this example, we found fonts with no outlines for
// printable characters such as `A', i.e., `outline.n_contours' and
// `outline.n_points' were zero.  FT_Outline_Check() returned `true'.
// FT_Outline_Decompose() also returned `true' without walking the outline.
// That is, we had no way of knowing whether the outline existed and could
// be (or was) decomposed.  Therefore, we implemented this workaround to
// check whether the outline does exist and can be decomposed.
bool
OutlinePrinter::OutlineExists() const
{
  FT_Face          face = m_Face;
  FT_GlyphSlot     slot = face->glyph;
  FT_Outline   &outline = slot->outline;
  
  if (slot->format != FT_GLYPH_FORMAT_OUTLINE)
    return false; // Should never happen.  Just an extra check.
  
  if (outline.n_contours <= 0 || outline.n_points <= 0)
    return false; // Can happen for some font files.
  
  FT_Error error = FT_Outline_Check(&outline);
  
  return error == 0;
}


// FreeType and SVG use opposite vertical directions.
void
OutlinePrinter::FlipOutline() const
{
  const FT_Fixed multiplier = 65536L;
  
  FT_Matrix matrix;
  
  matrix.xx =  1L * multiplier;
  matrix.xy =  0L * multiplier;
  matrix.yx =  0L * multiplier;
  matrix.yy = -1L * multiplier;
  
  FT_Face          face = m_Face;
  FT_GlyphSlot     slot = face->glyph;
  FT_Outline   &outline = slot->outline;
  
  FT_Outline_Transform(&outline, &matrix);
}

#define SVG_INDENT "" //"           "

void
OutlinePrinter::ExtractOutline()
{
//  m_path << "  <path d=\"";
  
  FT_Outline_Funcs callbacks;
  
  callbacks.move_to  = moveTo;
  callbacks.line_to  = lineTo;
  callbacks.conic_to = conicTo;
  callbacks.cubic_to = cubicTo;
  
  callbacks.shift    = 0;
  callbacks.delta    = 0;
  
  FT_Face           face = m_Face;
  FT_GlyphSlot      slot = face->glyph;
  FT_Outline    &outline = slot->outline;
  
  FT_Error error = FT_Outline_Decompose(&outline, &callbacks, this);
  
  if (error)
    throw runtime_error("Couldn't extract the outline:"
                        " FT_Outline_Decompose() failed");
  
  
//  m_path << "\"/>\n";
//            "        fill=\"#FF0000\"/>\n";
}


void
OutlinePrinter::ComputeViewBox()
{
  FT_Face           face = m_Face;
  FT_GlyphSlot      slot = face->glyph;
  FT_Outline    &outline = slot->outline;
  
  FT_BBox boundingBox;
  
  FT_Outline_Get_BBox(&outline, &boundingBox);
  
  FT_Pos xMin = boundingBox.xMin;
  FT_Pos yMin = boundingBox.yMin;
  FT_Pos xMax = boundingBox.xMax;
  FT_Pos yMax = boundingBox.yMax;
  
  m_xMin   = xMin;
  m_yMin   = yMin;
  m_width  = xMax - xMin;
  m_height = yMax - yMin;
}


void
OutlinePrinter::PrintSVG() const
{
  cout << "<svg xmlns=\"http://www.w3.org/2000/svg\"\n"
          "     xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
          "     viewBox=\""
          << m_xMin << ' ' << m_yMin << ' ' << m_width << ' ' << m_height
          << "\">\n"
          << m_path.str()
          << "</svg>"
          << endl;
}


int
OutlinePrinter::moveTo(const FT_Vector *to,
                               void *user)
{
  OutlinePrinter *self = static_cast<OutlinePrinter *>(user);
  
  FT_Pos x = to->x;
  FT_Pos y = to->y;
  
  self->m_path << SVG_INDENT
    "M " << x << ' ' << y << '\n';
  
  return 0;
}


int
OutlinePrinter::lineTo(const FT_Vector *to,
                               void *user)
{
  OutlinePrinter *self = static_cast<OutlinePrinter *>(user);
  
  FT_Pos x = to->x;
  FT_Pos y = to->y;
  
  self->m_path << SVG_INDENT
    "L " << x << ' ' << y << '\n';
  
  return 0;
}


int
OutlinePrinter::conicTo(const FT_Vector *pt,
                        const FT_Vector *to,
                        void *user)
{
  OutlinePrinter *self = static_cast<OutlinePrinter *>(user);
  
  FT_Pos controlX = pt->x;
  FT_Pos controlY = pt->y;
  
  FT_Pos x = to->x;
  FT_Pos y = to->y;
  
  self->m_path << SVG_INDENT
    "Q " << controlX << ' ' << controlY << ", "
    << x << ' ' << y << '\n';
  
  return 0;
}


int
OutlinePrinter::cubicTo(const FT_Vector *pt1,
                        const FT_Vector *pt2,
                        const FT_Vector *to,
                        void *user)
{
  OutlinePrinter *self = static_cast<OutlinePrinter *>(user);
  
  FT_Pos pt1X = pt1->x;
  FT_Pos pt1Y = pt1->y;
  
  FT_Pos pt2X = pt2->x;
  FT_Pos pt2Y = pt2->y;
  
  FT_Pos x = to->x;
  FT_Pos y = to->y;
  
  self->m_path << SVG_INDENT
    "C " << pt1X << ' ' << pt1Y << ", "
    << pt2X << ' ' << pt2Y << ", "
    << x << ' ' << y << '\n';
  
  return 0;
}

rtString
OutlinePrinter::getRawPath()
{
  return m_path.str().c_str();
}

rtString
OutlinePrinter::getPath()
{
  ostringstream svg;
  
  svg << "<svg xmlns=\"http://www.w3.org/2000/svg\"\n"
         "     xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
         "     viewBox=\""
         << m_xMin << ' ' << m_yMin << ' ' << m_width << ' ' << m_height
         << "\">\n"
         << "  <path d=\""
         << m_path.str()
         << "\"/>\n"
         << "</svg>"
         << endl;
  
  return svg.str().c_str();
}

/**********************************************************************/
/**********************************************************************/

/** Public API exposed to java script **/
rtError pxFont::getPathSVG(rtString text, rtValue& o)
{
  OutlinePrinter op(mFace);
  
  op.GlyphToSvg( text.cString() );

  o = op.getPath();
  
  return RT_OK;
}

/** Public API exposed to java script **/
rtError pxFont::getRawPathSVG(rtString text, rtValue& o)
{
  OutlinePrinter op(mFace);
  
  op.GlyphToSvg( text.cString() );
  
  o = op.getRawPath();
  
  return RT_OK;
}


/**********************************************************************/
/**********************************************************************/

// pxTextMetrics
rtDefineObject(pxTextMetrics, rtObject);
rtDefineProperty(pxTextMetrics, height); 
rtDefineProperty(pxTextMetrics, ascent);
rtDefineProperty(pxTextMetrics, descent);
rtDefineProperty(pxTextMetrics, naturalLeading);
rtDefineProperty(pxTextMetrics, baseline);

// pxFont
rtDefineObject(pxFont, pxResource);
rtDefineMethod(pxFont, getFontMetrics);
rtDefineMethod(pxFont, measureText);
rtDefineMethod(pxFont, getRawPathSVG);
rtDefineMethod(pxFont, getPathSVG);

rtDefineObject(pxTextSimpleMeasurements, rtObject);
rtDefineProperty(pxTextSimpleMeasurements, w);
rtDefineProperty(pxTextSimpleMeasurements, h);

#ifdef PXSCENE_FONT_ATLAS
#define PXSCENE_FONT_ATLAS_DIM 2048
pxFontAtlas::pxFontAtlas(): fence(0)
{
  mTexture = context.createTexture(PXSCENE_FONT_ATLAS_DIM,PXSCENE_FONT_ATLAS_DIM,PXSCENE_FONT_ATLAS_DIM,PXSCENE_FONT_ATLAS_DIM, NULL);
}

void pxFontAtlas::clearTexture() 
{
  if( mTexture) {
    mTexture->deleteTexture();
    mTexture = 0;
  }
}
bool pxFontAtlas::addGlyph(uint32_t w, uint32_t h, void* buffer, GlyphTextureEntry& e)
{
  //return false;
  // bail on biggish glyphs
  if (h < 128)
  {
    uint32_t h2 = (h+8)&~7;
    // look for a row that fits
    uint32_t i;
    for (i = 0; i < mRows.size(); i++)
    {
      row& r = mRows[i];
      if ((h2 == r.height) && (r.rFence+w < (uint32_t)mTexture->width()))
      {
        e.t = mTexture;
        e.u1 = (float)r.rFence/(float)mTexture->width();
        e.u2 = (float)(r.rFence+w)/(float)mTexture->width();
        e.v1 = (float)r.top/(float)mTexture->height();
        e.v2 = (float)(r.top+h)/(float)mTexture->height();
        
        mTexture->updateTexture(r.rFence,r.top,w,h,buffer);

        r.rFence += w+1;

        return true;
      }
    }

    if ((i >= mRows.size()) && (fence+h < (uint32_t)mTexture->height()))
    {
      // Didn't find a row that matched... try adding another row
      row nr;
      nr.top = fence;
      nr.height = h2;
      nr.rFence = 0;
      fence += h2;


      if ((h2 == nr.height) && (nr.rFence+w < (uint32_t)mTexture->width()))
      {
        e.t = mTexture;
        e.u1 = (float)nr.rFence/(float)mTexture->width();
        e.u2 = (float)(nr.rFence+w)/(float)mTexture->width();
        e.v1 = (float)nr.top/(float)mTexture->height();
        e.v2 = (float)(nr.top+h)/(float)mTexture->height(); 

        mTexture->updateTexture(nr.rFence,nr.top,w,h,buffer);

        nr.rFence += w+1;

        mRows.push_back(nr);
        return true;
      }        
    }
  }
  return false;
}


void pxTexturedQuads::draw(float x, float y, float* color)
{
  for (uint32_t i = 0; i < mQuads.size(); i++)
  {
    quads& q = mQuads[i];
    vector<float> verts(q.verts);

    if (x!= 0 || y != 0)
    {
      for (uint32_t j = 0; j < verts.size()/2; j++)
      {
        // offset x coords
        verts[(j*2)] += x;
        // offset y coords
        verts[(j*2)+1] += y;
      }
    }

    context.drawTexturedQuads(q.verts.size()/12, &verts[0], &q.uvs[0], q.t, color);
  }
}
#endif
