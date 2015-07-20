// pxCore CopyRight 2007-2015 John Robinson
// pxText.cpp

#include "pxText.h"
#include "pxFileDownloader.h"
#include "pxTimer.h"

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


// Weak Map
typedef map<rtString, pxFace*> FaceMap;
FaceMap gFaceMap;

uint32_t gFaceId = 0;

pxFace::pxFace():mRefCount(0) { mFaceId = gFaceId++; }

rtError pxFace::init(const char* n)
{
  if(FT_New_Face(ft, n, 0, &mFace))
    return RT_FAIL;
  
  mFaceName = n;
  setPixelSize(defaultPixelSize);
  gFaceMap.insert(make_pair(n, this));
  return RT_OK;
}

rtError pxFace::init(const FT_Byte*  fontData, FT_Long size, const char* n)
{
  if(FT_New_Memory_Face(ft, fontData, size, 0, &mFace))
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
      rtLogDebug("glyph cache miss");
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
      h += (metrics->height);
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

pxText::pxText(pxScene2d* s):pxObject(s), mFontDownloadRequest(NULL)
{
  initFT();
  float c[4] = {1, 1, 1, 1};
  memcpy(mTextColor, c, sizeof(mTextColor));
  mFace = gFace;
  mPixelSize = defaultPixelSize;
  mDirty = true;
}

pxText::~pxText()
{
  if (mFontDownloadRequest != NULL)
  {
    // if there is a previous request pending then set the callback to NULL
    // the previous request will not be processed and the memory will be freed when the download is complete
    mFontDownloadRequest->setCallbackFunctionThreadSafe(NULL);
  }
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


        
void pxText::update(double t)
{
  pxObject::update(t);
  
#if 1
  if (mDirty)
  {
#if 0
    // TODO magic number
    if (mText.length() >= 5)
    {
      setPainting(true);
      setPainting(false);
    }
    else
      setPainting(true);
#else
    if (mText.length() >= 5)
    {
      mCached = NULL;
      pxTextureRef cached = context.createContextSurface(mw,mh);
      if (cached.getPtr())
      {
        pxTextureRef previousSurface = context.getCurrentRenderSurface();
        cached->enablePremultipliedAlpha(true);
        context.setRenderSurface(cached);
        pxMatrix4f m;
        context.setMatrix(m);
        context.setAlpha(ma);
        context.clear(mw,mh);
        draw();
        context.setRenderSurface(previousSurface);
        mCached = cached;
      }
    }
    else mCached = NULL;
    
#endif
    
    mDirty = false;
    }
#else
  mDirty = false;
#endif
  
}

void pxText::draw() {
  if (mCached.getPtr())
    context.drawImage(0,0,mw,mh,mCached,pxTextureRef(),PX_NONE,PX_NONE);
  else
    mFace->renderText(mText, mPixelSize, 0, 0, 1.0, 1.0, mTextColor, mw);
}

rtError pxText::setFaceURL(const char* s)
{
  if (!s || !s[0])
    s = defaultFace;

  FaceMap::iterator it = gFaceMap.find(s);
  if (it != gFaceMap.end())
  {
    mReady.send("resolve", this);
    mFace = it->second;
  }
  else
  {
    const char *result = strstr(s, "http");
    int position = result - s;
    if (position == 0 && strlen(s) > 0)
    {
      if (mFontDownloadRequest != NULL)
      {
        // if there is a previous request pending then set the callback to NULL
        // the previous request will not be processed and the memory will be freed when the download is complete
        mFontDownloadRequest->setCallbackFunctionThreadSafe(NULL);
      }
      mFontDownloadRequest =
          new pxFileDownloadRequest(s, this);
      fontDownloadsPending++;
      mFontDownloadRequest->setCallbackFunction(pxFontDownloadComplete);
      pxFileDownloader::getInstance()->addToDownloadQueue(mFontDownloadRequest);
    }
    else {
      pxFaceRef f = new pxFace;
      rtError e = f->init(s);
      if (e != RT_OK)
      {
        rtLogInfo("Could not load font face, %s, %s\n", "blah", s);
        mReady.send("reject",this);
        return e;
      }
      else
      {
        mReady.send("resolve", this);
        mFace = f;
      }
    }
  }
  mFaceURL = s;
  
  return RT_OK;
}

void pxText::checkForCompletedDownloads(int maxTimeInMilliseconds)
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
          pxText *textObject = (pxText *) fontDownloadRequest.fileDownloadRequest->getCallbackData();
          textObject->onFontDownloadComplete(fontDownloadRequest);
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

void pxText::onFontDownloadComplete(FontDownloadRequest fontDownloadRequest)
{
  mFontDownloadRequest = NULL;
  if (fontDownloadRequest.fileDownloadRequest == NULL)
  {
    mReady.send("reject",this);
    return;
  }
  if (fontDownloadRequest.fileDownloadRequest->getDownloadStatusCode() == 0 &&
      fontDownloadRequest.fileDownloadRequest->getHttpStatusCode() == 200 &&
      fontDownloadRequest.fileDownloadRequest->getDownloadedData() != NULL)
  {
    pxFaceRef f = new pxFace;
    rtError e = f->init((FT_Byte*)fontDownloadRequest.fileDownloadRequest->getDownloadedData(),
                        (FT_Long)fontDownloadRequest.fileDownloadRequest->getDownloadedDataSize(),
                        fontDownloadRequest.fileDownloadRequest->getFileURL().cString());
    if (e != RT_OK)
    {
      rtLogInfo("Could not load font face, %s, %s\n", "blah", fontDownloadRequest.fileDownloadRequest->getFileURL().cString());
      mReady.send("reject",this);
    }
    else {
      mFace = f;
      mReady.send("resolve",this);
    }
  }
  else
  {
    rtLogWarn("Font Download Failed: %s Error: %s HTTP Status Code: %ld",
              fontDownloadRequest.fileDownloadRequest->getFileURL().cString(),
              fontDownloadRequest.fileDownloadRequest->getErrorString().cString(),
              fontDownloadRequest.fileDownloadRequest->getHttpStatusCode());
    if (mParent != NULL)
    {
      mReady.send("reject",this);
    }
  }
}

rtDefineObject(pxText, pxObject);
rtDefineProperty(pxText, text);
rtDefineProperty(pxText, textColor);
rtDefineProperty(pxText, pixelSize);
rtDefineProperty(pxText, faceURL);
