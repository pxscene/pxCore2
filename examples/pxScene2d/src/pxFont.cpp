/*

 pxCore Copyright 2005-2017 John Robinson

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
typedef map<GlyphKey,pxTextureRef> GlyphTextureCache;

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
uint32_t gFontId = 0;

// TODO move out to rt* utility
uint32_t npot(uint32_t i)
{
  uint32_t power = 1;
  while(power < i)
    power*=2;
  return power;
}

pxFont::pxFont(rtString fontUrl, rtString proxyUrl):pxResource(),mFace(NULL),mPixelSize(0), mFontData(0), mFontDataSize(0),
             mFontMutex()
{  
  mFontId = gFontId++; 
  mUrl = fontUrl;
  mProxy = proxyUrl;
}

pxFont::~pxFont() 
{
  rtLogInfo("~pxFont %s\n", mUrl.cString());
  gUIThreadQueue.removeAllTasksForObject(this);
  // download should be canceled/removed in pxResource
  //if (mDownloadRequest != NULL)
  //{
    //// clear any pending downloads
    //mFontDownloadRequest->setCallbackFunctionThreadSafe(NULL);
  //}  
   
  pxFontManager::removeFont( mUrl);
 
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
   
}

bool pxFont::loadResourceData(rtFileDownloadRequest* fileDownloadRequest)
{
      // Load the font data
      init( (FT_Byte*)fileDownloadRequest->downloadedData(), 
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
      mLoadStatus.set("statusCode", PX_RESOURCE_STATUS_FILE_NOT_FOUND);
      // Since this object can be released before we get a async completion
      // We need to maintain this object's lifetime
      // TODO review overall flow and organization
      AddRef();     
      gUIThreadQueue.addTask(onDownloadCompleteUI, this, (void*)"reject");
    }
    else
    {
      mLoadStatus.set("statusCode", PX_RESOURCE_STATUS_OK);
      // Since this object can be released before we get a async completion
      // We need to maintain this object's lifetime
      // TODO review overall flow and organization
      AddRef();      
      gUIThreadQueue.addTask(onDownloadCompleteUI, this, (void*)"resolve");

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

pxTextureRef pxFont::getGlyphTexture(uint32_t codePoint, float sx, float sy)
{
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
      
      pxTextureRef texture = context.createTexture(g->bitmap.width, g->bitmap.rows, 
                                              g->bitmap.width, g->bitmap.rows, 
                                              g->bitmap.buffer);
      
      gGlyphTextureCache.insert(make_pair(key,texture));

      // restore current pixelSize
      FT_Set_Pixel_Sizes(mFace, 0, mPixelSize);
      return texture;  
    }
    // restore current pixelSize
    FT_Set_Pixel_Sizes(mFace, 0, mPixelSize);
  }
  return NULL;  
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
//    float y2 = y - g->bitmap_top;
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
      
      //pxTextureRef texture = entry->mTexture;
      pxTextureRef texture = getGlyphTexture(codePoint, nsx, nsy);
      pxTextureRef nullImage;
      context.drawImage(x2,y2, w, h, texture, nullImage, false, color);
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

/** Public API exposed to java script */
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
/**                    pxFontManager                                  */
/**********************************************************************/
FontMap pxFontManager::mFontMap;
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

  if (!url || !url[0])
    url = defaultFont;
  
  FontMap::iterator it = mFontMap.find(url);
  if (it != mFontMap.end())
  {
    rtLogDebug("Found pxFont in map for %s\n",url);
    pFont = it->second;
    return pFont;  
    
  }
  else 
  {
    rtLogDebug("Create pxFont in map for %s\n",url);
    pFont = new pxFont(url, proxy);
    mFontMap.insert(make_pair(url, pFont));
    pFont->loadResource();
  }
  
  return pFont;
}

void pxFontManager::removeFont(rtString fontName)
{
  FontMap::iterator it = mFontMap.find(fontName);
  if (it != mFontMap.end())
  {  
    mFontMap.erase(it);
  }
}

void pxFontManager::clearAllFonts()
{
  rtLogInfo(__FUNCTION__);
  for (GlyphCache::iterator it =  gGlyphCache.begin(); it != gGlyphCache.end(); it++)
    delete it->second;

  gGlyphCache.clear();
  gGlyphTextureCache.clear();

  rtLogInfo("Finished in pxFontManager::clearAllFonts()");
}

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

rtDefineObject(pxTextSimpleMeasurements, rtObject);
rtDefineProperty(pxTextSimpleMeasurements, w);
rtDefineProperty(pxTextSimpleMeasurements, h);
