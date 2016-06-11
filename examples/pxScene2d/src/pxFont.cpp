// pxCore CopyRight 2007-2015 John Robinson
// pxFont.cpp

#include "pxFont.h"
#include "pxFileDownloader.h"
#include "pxTimer.h"
#include "pxText.h"

#include <math.h>
#include <map>

struct GlyphKey {
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

GlyphCache gGlyphCache;

#include "pxContext.h"

extern pxContext context;

// TODO can we eliminate direct utf8.h usage
extern "C" {
#include "utf8.h"
}

#if 0
<link href='http://fonts.googleapis.com/css?family=Fontdiner+Swanky' rel='stylesheet' type='text/css'>
#endif

FT_Library ft;
uint32_t gFontId = 0;


pxFont::pxFont(rtString fontUrl):pxResource(),mPixelSize(0), mFontData(0)
{  
  mFontId = gFontId++; 
  mUrl = fontUrl;

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
  } 
   
}
bool pxFont::loadResourceData(pxFileDownloadRequest* fileDownloadRequest)
{
      // Load the font data
      init( (FT_Byte*)fileDownloadRequest->getDownloadedData(), 
            (FT_Long)fileDownloadRequest->getDownloadedDataSize(), 
            fileDownloadRequest->getFileUrl().cString());
            
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

rtError pxFont::init(const char* n)
{
  mUrl = n;
    
  if(FT_New_Face(ft, n, 0, &mFace))
    return RT_FAIL;
  
  mInitialized = true;
  setPixelSize(defaultPixelSize);

  return RT_OK;
}

rtError pxFont::init(const FT_Byte*  fontData, FT_Long size, const char* n)
{
  // We need to keep a copy of fontData since the download will be deleted.
  mFontData = (char *)malloc(size);
  memcpy(mFontData, fontData, size);
  
  if(FT_New_Memory_Face(ft, (const FT_Byte*)mFontData, size, 0, &mFace))
    return RT_FAIL;

  mUrl = n;
  mInitialized = true;
  setPixelSize(defaultPixelSize);
  
  return RT_OK;
}


void pxFont::setPixelSize(uint32_t s)
{
  if (mPixelSize != s && mInitialized)
  {
    //printf("pxFont::setPixelSize size=%d mPixelSize=%d mInitialized=%d and mFace=%d\n", s,mPixelSize,mInitialized, mFace);
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
      entry->advancedotx = g->advance.x;
      entry->advancedoty = g->advance.y;
      entry->vertAdvance = g->metrics.vertAdvance; // !CLF: Why vertAdvance? SHould only be valid for vert layout of text.
      
      entry->mTexture = context.createTexture(g->bitmap.width, g->bitmap.rows, 
                                              g->bitmap.width, g->bitmap.rows, 
                                              g->bitmap.buffer);
      
      gGlyphCache.insert(make_pair(key,entry));
      return entry;
    }
  }
  return NULL;
}

void pxFont::measureTextInternal(const char* text, uint32_t size,  float sx, float sy, 
                         float& w, float& h) 
{
  if( !mInitialized) 
  {
    rtLogWarn("measureText called TOO EARLY -- not initialized or font not loaded!\n");
    return;
  }

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
      h += (metrics->height>>6) *sy;
      lw = 0;
    }
    w = pxMax<float>(w, lw);
//    h = pxMax<float>((g->advance.y >> 6) * sy, h);
//    h = pxMax<float>((metrics->height >> 6) * sy, h);
  }
  h *= sy;
}


uint32_t npot(uint32_t i)
{
  uint32_t power = 1;
  while(power < i)
    power*=2;
  return power;
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

  // TODO could be better performing....
  // TODO probably should be the max of nsx and nsy
  uint32_t size2 = npot(floor((double)size*nsx));
  setPixelSize(size2);

  float sx = (((double)size*nsx)/(float)size2)/nsx;
  float sy = sx; // TODO should calculate this separately

  FT_Size_Metrics* metrics = &mFace->size->metrics;

  while((codePoint = u8_nextchar((char*)text, &i)) != 0) 
  {
    const GlyphCacheEntry* entry = getGlyph(codePoint);
    if (!entry) 
      continue;

    float x2 = (float)x + (float)entry->bitmap_left * sx;
//    float y2 = y - g->bitmap_top * sy;
    float y2 = (float)((y - entry->bitmap_top) + (metrics->ascender>>6)) * sy;
    float w = (float)entry->bitmapdotwidth * sx;
    float h = (float)entry->bitmapdotrows * sy;
    
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
      context.drawImage(x2,y2, w, h, texture, nullImage, false, color);
      x += (entry->advancedotx >> 6) * sx;
      // no change to y because we are not moving to next line yet
    }
    else
    {
      x = 0;
      // Use height to advance to next line
      y += (metrics->height>>6) * sy;
    }
  }
}

void pxFont::measureTextChar(u_int32_t codePoint, uint32_t size,  float sx, float sy, 
                         float& w, float& h) 
{
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
  //printf("pxFont::getFontMetrics\n");  
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
  //printf("pxFont::measureText returned %f and %f for pixelSize=%d and text \"%s\"\n",w, h,pixelSize, stringToMeasure.cString());
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
    //printf("initFT returning; already inited\n");
    return;
  }
  init = true;
  
  if(FT_Init_FreeType(&ft)) 
  {
    rtLogError("Could not init freetype library\n");
    return;
  }
  
}
rtRefT<pxFont> pxFontManager::getFont(const char* url)
{
  initFT();

  rtRefT<pxFont> pFont;

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
    pFont = new pxFont(url);
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


// pxTextMetrics
rtDefineObject(pxTextMetrics, pxResource);
rtDefineProperty(pxTextMetrics, height); 
rtDefineProperty(pxTextMetrics, ascent);
rtDefineProperty(pxTextMetrics, descent);
rtDefineProperty(pxTextMetrics, naturalLeading);
rtDefineProperty(pxTextMetrics, baseline);

// pxFont
rtDefineObject(pxFont, pxResource);
rtDefineMethod(pxFont, getFontMetrics);
rtDefineMethod(pxFont, measureText);

rtDefineObject(pxTextSimpleMeasurements, pxResource);
rtDefineProperty(pxTextSimpleMeasurements, w);
rtDefineProperty(pxTextSimpleMeasurements, h);
