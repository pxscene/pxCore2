// pxCore CopyRight 2007-2015 John Robinson
// pxFont.cpp

#include "pxFont.h"
#include "pxFileDownloader.h"
#include "pxTimer.h"
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

// TODO can we eliminate direct utf8.h usage
extern "C" {
#include "utf8.h"
}

#if 0
<link href='http://fonts.googleapis.com/css?family=Fontdiner+Swanky' rel='stylesheet' type='text/css'>
#endif

FT_Library ft;


int fontDownloadsPending = 0; //must only be set in the main thread
rtMutex fontDownloadMutex;
bool fontDownloadsAvailable = false;
vector<FontDownloadRequest> completedFontDownloads;

void pxFontDownloadComplete(pxFileDownloadRequest* fileDownloadRequest)
{
  if (fileDownloadRequest != NULL)
  {
    FontDownloadRequest fontDownloadRequest;
    fontDownloadRequest.fileDownloadRequest = fileDownloadRequest;
    fontDownloadMutex.lock();
    completedFontDownloads.push_back(fontDownloadRequest);
    fontDownloadsAvailable = true;
    fontDownloadMutex.unlock();
  }
}

uint32_t gFaceId = 0;

void pxFont::onDownloadComplete(const FT_Byte*  fontData, FT_Long size, const char* n)
{
  rtLogInfo("pxFont::onDownloadComplete %s\n",n);
  if( mFaceName.compare(n)) 
  {
    rtLogWarn("pxFont::onDownloadComplete received for face \"%s\" but this face is \"%s\"\n",n, mFaceName.cString());
    return; 
  }

  init(fontData, size, n);
  
  fontLoaded();

}


rtError pxFont::init(const char* n)
{
  mFaceName = n;
    
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

  mFaceName = n;
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
    rtLogWarn("getMetrics called on font before it is initialized\n");
    return;
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

void pxFont::measureText(const char* text, uint32_t size,  float sx, float sy, 
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
      h += metrics->height>>6;
      lw = 0;
    }
    w = pxMax<float>(w, lw);
//    h = pxMax<float>((g->advance.y >> 6) * sy, h);
//    h = pxMax<float>((metrics->height >> 6) * sy, h);
  }
  h *= sy;
}

void pxFont::renderText(const char *text, uint32_t size, float x, float y, 
                        float sx, float sy, 
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


pxFont::pxFont(pxScene2d* scene, rtString faceUrl):pxObject(scene), mPixelSize(0), mFontData(0), mInitialized(false), mFontDownloadRequest(NULL)
{  
  mFaceId = gFaceId++; 
  mFaceName = faceUrl;
  
}

pxFont::~pxFont() 
{
  rtLogInfo("~pxFont %s\n", mFaceName.cString());
  if (mFontDownloadRequest != NULL)
  {
    // clear any pending downloads
    mFontDownloadRequest->setCallbackFunctionThreadSafe(NULL);
  }  
   
  pxFontManager::removeFont( mFaceName);
 
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

void pxFont::fontLoaded()
{
    mInitialized = true;
    sendReady("resolve");
}

void pxFont::addListener(pxText* pText) 
{
  //printf("pxFont::addListener for %s\n",mFaceName.cString());
  if( !mInitialized) 
  {
    mListeners.push_back(pText);
  } 
  else 
  {
    pText->fontLoaded("resolve");
  }
  
}
rtError pxFont::loadFont()
{
  rtLogInfo("pxFont::loadFont for %s\n",mFaceName.cString());
  const char *result = strstr(mFaceName, "http");
  int position = result - mFaceName;
  if (position == 0 && strlen(mFaceName) > 0)
  {
    if (mFontDownloadRequest != NULL)
    {
      // if there is a previous request pending then set the callback to NULL
      // the previous request will not be processed and the memory will be freed when the download is complete
      mFontDownloadRequest->setCallbackFunctionThreadSafe(NULL);
    }
    // Start the download request
    mFontDownloadRequest =
        new pxFileDownloadRequest(mFaceName, this);
   
    fontDownloadsPending++;
    mFontDownloadRequest->setCallbackFunction(pxFontDownloadComplete);
    pxFileDownloader::getInstance()->addToDownloadQueue(mFontDownloadRequest);

  }
  else {
    rtError e = init(mFaceName);
    if (e != RT_OK)
    {
      rtLogWarn("Could not load font face %s\n", mFaceName.cString());
      sendReady("reject");

      return e;
    }
    else
    {
      fontLoaded();

    }
  }
  
  return RT_OK;
}

void pxFont::checkForCompletedDownloads(int maxTimeInMilliseconds)
{
  double startTimeInMs = pxMilliseconds();
  if (fontDownloadsPending > 0)
  {
    fontDownloadMutex.lock();
    if (fontDownloadsAvailable)
    {
      for(vector<FontDownloadRequest>::iterator it = completedFontDownloads.begin(); it != completedFontDownloads.end(); )
      {
        FontDownloadRequest fontDownloadRequest = (*it);
        if (!fontDownloadRequest.fileDownloadRequest)
        {
          it = completedFontDownloads.erase(it);
          continue;
        }
        if (fontDownloadRequest.fileDownloadRequest->getCallbackData() != NULL)
        {
          pxFont *fontObject = (pxFont *) fontDownloadRequest.fileDownloadRequest->getCallbackData();
          fontObject->onFontDownloadComplete(fontDownloadRequest);
        }

        delete fontDownloadRequest.fileDownloadRequest;
        fontDownloadsAvailable = false;
        fontDownloadsPending--;
        it = completedFontDownloads.erase(it);
        double currentTimeInMs = pxMilliseconds();
        if ((maxTimeInMilliseconds >= 0) && (currentTimeInMs - startTimeInMs > maxTimeInMilliseconds))
        {
          break;
        }
      }
      if (fontDownloadsPending < 0)
      {
        //this is a safety check (hopefully never used)
        //to ensure downloads are still processed in the event of a fontDownloadsPending bug in the future
        fontDownloadsPending = 0;
      }
    }
    fontDownloadMutex.unlock();
  }
}

void pxFont::onFontDownloadComplete(FontDownloadRequest fontDownloadRequest)
{
  rtLogInfo("pxFont::onFontDownloadComplete\n");
  mFontDownloadRequest = NULL;
  if (fontDownloadRequest.fileDownloadRequest == NULL)
  {
    sendReady("reject");
    return;
  }
  if (fontDownloadRequest.fileDownloadRequest->getDownloadStatusCode() == 0 &&
      fontDownloadRequest.fileDownloadRequest->getHttpStatusCode() == 200 &&
      fontDownloadRequest.fileDownloadRequest->getDownloadedData() != NULL)
  {
    // Let the face handle the completion event and notifications to listeners
    onDownloadComplete((FT_Byte*)fontDownloadRequest.fileDownloadRequest->getDownloadedData(),
                          (FT_Long)fontDownloadRequest.fileDownloadRequest->getDownloadedDataSize(),
                          fontDownloadRequest.fileDownloadRequest->getFileUrl().cString());

  }
  else
  {
    rtLogWarn("Font Download Failed: %s Error: %s HTTP Status Code: %ld",
              fontDownloadRequest.fileDownloadRequest->getFileUrl().cString(),
              fontDownloadRequest.fileDownloadRequest->getErrorString().cString(),
              fontDownloadRequest.fileDownloadRequest->getHttpStatusCode());
    if (mParent != NULL)
    {
      sendReady("reject");
    }
  }
}
void pxFont::sendReady(const char * value)
{
  for (vector<pxText*>::iterator it = mListeners.begin();
         it != mListeners.end(); ++it)
  {
    (*it)->fontLoaded(value);

  }
  mListeners.clear();
  mReady.send(value,this);
}

/*
#### getFontMetrics - returns information about the font face (font and size).  It does not convey information about the text of the font.  
* See section 3.a in http://www.freetype.org/freetype2/docs/tutorial/step2.html .  
* The returned object has the following properties:
* height - float - the distance between baselines
* ascent - float - the distance from the baseline to the font ascender (note that this is a hint, not a solid rule)
* descent - float - the distance from the baseline to the font descender  (note that this is a hint, not a solid rule)
*/
rtError pxFont::getFontMetrics(uint32_t pixelSize, rtObjectRef& o) 
{
  //printf("pxText2::getFontMetrics\n");  
	float height, ascent, descent, naturalLeading;
	pxTextMetrics* metrics = new pxTextMetrics(mScene);

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
  metrics->setBaseline(my+ascent);
	o = metrics;

	return RT_OK;
}

rtError pxFont::measureText(uint32_t pixelSize, rtString stringToMeasure, rtObjectRef& o)
{
    pxTextSimpleMeasurements* measure = new pxTextSimpleMeasurements(mScene);
    
    if(!mInitialized ) 
    {
      rtLogWarn("measureText called TOO EARLY -- not initialized or font not loaded!\n");
      o = measure;
      return RT_OK; // !CLF: TO DO - COULD RETURN RT_ERROR HERE TO CATCH NOT WAITING ON PROMISE
    } 
    float w, h;
    measureText(stringToMeasure, pixelSize, 1.0,1.0, w, h);
    measure->setW(w);
    measure->setH(h);
    o = measure;
    
    return RT_OK; 
}

FontMap pxFontManager::mFontMap;
bool pxFontManager::init = false;
void pxFontManager::initFT(pxScene2d* scene) 
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
  
  //// Set up default font
  //rtRefT<pxFont> pFont = new pxFont(scene, defaultFace);
  //mFontMap.insert(make_pair(defaultFace, pFont));

  //pFont->loadFont();

}
rtRefT<pxFont> pxFontManager::getFont(pxScene2d* scene, const char* s)
{
  initFT(scene);

  rtRefT<pxFont> pFont;

  if (!s || !s[0])
    s = defaultFace;
  
  FontMap::iterator it = mFontMap.find(s);
  if (it != mFontMap.end())
  {
    rtLogDebug("Found pxFont in map for %s\n",s);
    pFont = it->second;
    return pFont;  
    
  }
  else 
  {
    rtLogDebug("Create pxFont in map for %s\n",s);
    pFont = new pxFont(scene, s);
    mFontMap.insert(make_pair(s, pFont));
    pFont->loadFont();
  }
  
  return pFont;
}

void pxFontManager::removeFont(rtString faceName)
{
  FontMap::iterator it = mFontMap.find(faceName);
  if (it != mFontMap.end())
  {  
    mFontMap.erase(it);
  }
}


// pxTextMetrics
rtDefineObject(pxTextMetrics, pxObject);
rtDefineProperty(pxTextMetrics, height); 
rtDefineProperty(pxTextMetrics, ascent);
rtDefineProperty(pxTextMetrics, descent);
rtDefineProperty(pxTextMetrics, naturalLeading);
rtDefineProperty(pxTextMetrics, baseline);

// pxFont
rtDefineObject(pxFont, pxObject);
rtDefineMethod(pxFont, getFontMetrics);
rtDefineMethod(pxFont, measureText);

rtDefineObject(pxTextSimpleMeasurements, pxObject);
