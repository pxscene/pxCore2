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

#include "rtPathUtils.h"
#include "rtFileDownloader.h"
#include "pxFont.h"
#include "pxTimer.h"
#include "pxEffects.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include <ft2build.h>
#include FT_FREETYPE_H

// Helper function to get 'FT_Error' message strings.
//
static const char* getErrorMessage(FT_Error err)
{
  #undef __FTERRORS_H__
  #define FT_ERRORDEF( e, v, s )  case e: return s;
  #define FT_ERROR_START_LIST     switch (err) {
  #define FT_ERROR_END_LIST       }
  #include FT_ERRORS_H
    return "(Unknown error)";
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <cmath>
#include <map>

#define  CODEPOINT_SPACE 0x20 // 32 // Space character

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

pxFont::pxFont(rtString fontUrl, uint32_t id, rtString proxyUrl, rtString fontStyle):pxResource(),mFace(NULL),mPixelSize(0), mFontData(0), mFontDataSize(0),
             mFontMutex(), mFontDataMutex(), mFontDownloadedData(NULL), mFontDownloadedDataSize(0),
            mFontDataUrl(), mFontStyle(), mFallbackFont(NULL)
{  
  mFontId = id; 
  mUrl = fontUrl;
  mProxy = proxyUrl;
  mFontStyle = fontStyle;
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

uint32_t pxFont::loadResourceData(rtFileDownloadRequest* fileDownloadRequest)
{
      // Load the font data
    setFontData( (FT_Byte*)fileDownloadRequest->downloadedData(),
            (FT_Long)fileDownloadRequest->downloadedDataSize(),
            fileDownloadRequest->fileUrl().cString());
            
      return PX_RESOURCE_LOAD_SUCCESS;
}

void pxFont::loadResourceFromArchive(rtObjectRef archiveRef)
{
  pxArchive* archive = (pxArchive*)archiveRef.getPtr();
  pxOffscreen imageOffscreen;
  rtData d;
  if ((NULL != archive) && (archive->getFileData(mUrl, d) == RT_OK))
  {
    init( (FT_Byte*)d.data(),
          (FT_Long)d.length(),
          mUrl.cString());
    setLoadStatus("statusCode", PX_RESOURCE_STATUS_OK);
    AddRef();
    if (gUIThreadQueue)
    {
      gUIThreadQueue->addTask(onDownloadCompleteUI, this, (void*)"resolve");
    }
    rtLogInfo("Resource [%s] loaded from archive successfully !!!",mUrl.cString());
  }
  else
  {
    rtLogWarn("Could not load font face from archive %s\n", mUrl.cString());
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
// Simulating italic or oblique font style if required (and possible)
bool pxFont::transform()
{
    bool res = false;
    rtString loadedStyleName(mFace->style_name);
    if (!loadedStyleName.isEmpty())loadedStyleName.toLowerAscii();
    if (!mFontStyle.isEmpty()) mFontStyle.toLowerAscii();

    do {
        if (loadedStyleName.beginsWith("italic"))
        {
            mFontStyle = loadedStyleName;
            break;
        }

        if (mFontStyle.isEmpty() || !(mFontStyle.beginsWith("italic") || mFontStyle.beginsWith("oblique")))
        {
            break;
        }

        double k = 0;

        if (mFontStyle.beginsWith("italic"))
        {
            k = 0.24;
        }
        else
        {
            int32_t pos = mFontStyle.find(0, " ");

            if (pos < 0) break;

            double angle = atof(mFontStyle.substring(pos, mFontStyle.length() - 3).cString());

            k = tan(angle * M_PI / 180);
        }

        if (k <= 0) break;

        FT_Matrix matrix;

        matrix.xx = 0x10000L;
        matrix.xy = k * 0x10000L;
        matrix.yx = 0;
        matrix.yy = 0x10000L;

        FT_Set_Transform(mFace, &matrix, 0);
        res = true;
        break;
    } while(0);

    if (mFontStyle.isEmpty()) {
        mFontStyle = mFace->style_name;
        mFontStyle.toLowerAscii();
    }

    return res;
}

// This init(char*) is for load of local font files
rtError pxFont::init(const char* n)
{
  mFontMutex.lock();
  mUrl = n;
  rtError loadFontStatus = RT_FAIL;

  rtString urlStr = mUrl.beginsWith("file://") ? mUrl.substring(7) : mUrl;

  do {
    if (FT_New_Face(ft, urlStr.cString(), 0, &mFace) == 0)
    {
      loadFontStatus = RT_OK;
      transform();
      break;
    }

    if (rtIsPathAbsolute(urlStr))
      break;

    rtModuleDirs *dirs = rtModuleDirs::instance();

    for (rtModuleDirs::iter it = dirs->iterator(); it.first != it.second; it.first++)
    {
      if (FT_New_Face(ft, rtConcatenatePath(*it.first, urlStr.cString()).c_str(), 0, &mFace) == 0)
      {
        loadFontStatus = RT_OK;
        transform();
        break;
      }
    }
  } while (0);

    if(loadFontStatus == RT_OK)
  {
    mInitialized = true;
    setPixelSize(defaultPixelSize);
  }

  mFontMutex.unlock();
  return loadFontStatus;
}
// This init is used by async callback to load downloaded font file data
rtError pxFont::init(const FT_Byte*  fontData, FT_Long size, const char* n)
{
  mFontMutex.lock();
  double startResourceSetupTime = pxMilliseconds();
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

  transform();
  mInitialized = true;
  setPixelSize(defaultPixelSize);

  double stopResourceSetupTime = pxMilliseconds();
  setLoadStatus("setupTimeMs", static_cast<int>(stopResourceSetupTime-startResourceSetupTime));

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
  	
	height = static_cast<float>(metrics->height>>6); 
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
  	
	height = static_cast<float>(metrics->height>>6);
	ascender =static_cast<float>(metrics->ascender>>6); 
	descender = static_cast<float>(-metrics->descender>>6); 
  naturalLeading = height - (ascender + descender);

}

GlyphTextureEntry pxFont::getGlyphTexture(uint32_t codePoint, float sx, float sy)
{
  FT_Error err  = 0;
  FT_Face *face = &mFace;

  GlyphTextureEntry result;
  // Select a glyph texture better suited for rendering the glyph
  // taking pixelSize and scale into account
  uint32_t pixelSize = (uint32_t) ceil( (sx > sy ? sx : sy) * mPixelSize );
  
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
  key.mFontId    = mFontId;
  key.mPixelSize = pixelSize; 
  key.mCodePoint = codePoint;

  GlyphTextureCache::iterator it = gGlyphTextureCache.find(key);
  if (it != gGlyphTextureCache.end())
  {
    return it->second; // FOUND in cache !!!
  }
  else
  {
    // temporarily set pixel size to more optimal size for rendering texture
    FT_Set_Pixel_Sizes(*face, 0, pixelSize);
    err = FT_Load_Char(*face, codePoint, FT_LOAD_RENDER);  // non-zero return ... means error - TODO only need to render glyph here - need 'FT_LOAD_RENDER' ?
    FT_Set_Pixel_Sizes(*face, 0, mPixelSize); // restore
  
    FT_GlyphSlot gg = (*face)->glyph;
    
    if(err != 0 || (codePoint != CODEPOINT_SPACE && (gg && gg->bitmap.width == 0)) ) // Not a SPACE,  Fallback for not found ?
    {
      // Try using the FALLBACK font
      pxFont *fallback = static_cast<pxFont *>( mFallbackFont.getPtr() );
      
      if(fallback)
      {
        face = &fallback->mFace;
        
        FT_Set_Pixel_Sizes(*face, 0, pixelSize);
        err = FT_Load_Char(*face, codePoint, FT_LOAD_RENDER);
        FT_Set_Pixel_Sizes(*face, 0, mPixelSize); // restore
      }
    }

    if(err == 0) // No error
    {
      rtLogDebug("glyph texture cache miss");

      FT_GlyphSlot g = (*face)->glyph;
      
#ifdef PXSCENE_FONT_ATLAS
      if (!gFontAtlas.addGlyph(g->bitmap.width, g->bitmap.rows, g->bitmap.buffer, result))
      {
#endif
        rtLogWarn("Glyph not in atlas");
        result.t = context.createTexture(static_cast<float>(g->bitmap.width), static_cast<float>(g->bitmap.rows),
                                         static_cast<float>(g->bitmap.width), static_cast<float>(g->bitmap.rows),
                                         g->bitmap.buffer);
        result.u1 = 0;
        result.v1 = 1;
        result.u2 = 1;
        result.v2 = 0;
#ifdef PXSCENE_FONT_ATLAS
      }
#endif
      
      gGlyphTextureCache.insert(make_pair(key, result));
    }
    else
    {
       rtLogDebug("getGlyphTexture() >>  FT_Load_Char() returned FT_Error = %d ... %s", err, getErrorMessage(err) );
    }
  }
  return result;  
}
  
const GlyphCacheEntry* pxFont::getGlyph(uint32_t codePoint)
{
  FT_Error err  = 0;
  FT_Face *face = &mFace;

  GlyphKey key; 
  key.mFontId    = mFontId;
  key.mPixelSize = mPixelSize; 
  key.mCodePoint = codePoint;

  GlyphCache::iterator it = gGlyphCache.find(key);
  if (it != gGlyphCache.end())
  {
    return it->second; // it's cached ... we're done !
  }
  else
  {
    // TODO should not need to 'FT_LOAD_RENDER' render here ???
    err = FT_Load_Char(*face, codePoint, FT_LOAD_RENDER);       // in the PRIMARY font ?
    
    FT_GlyphSlot gg = (*face)->glyph;

    if( (err != 0) ||                                                     // Not Found and/or Error
        (codePoint != CODEPOINT_SPACE && (gg && gg->bitmap.width == 0) )) // Not a SPACE ... but missing glyph !
    {
      // Try using the FALLBACK font
      pxFont *fallback = static_cast<pxFont *>( mFallbackFont.getPtr() );
      
      if(fallback)
      {
        face = &fallback->mFace;
        
        FT_Set_Pixel_Sizes(*face, 0, mPixelSize);
        err = FT_Load_Char(*face, codePoint, FT_LOAD_RENDER);     // in the FALLBACK font ?
      }

      if(err != 0)
      {
        rtLogDebug("getGlyph()  >> FT_Load_Char() returned FT_Error = %d ... %s try Fallback font ", err, (fallback ? "DID" : "did NOT"));
        rtLogDebug("getGlyph()  >> FT_Load_Char() returned FT_Error = %d ... %s", err, getErrorMessage(err));
        
        return NULL;
      }
    }
   
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Add "new" glyph to cache
    
    rtLogDebug("glyph cache miss");
    GlyphCacheEntry *entry = new GlyphCacheEntry();
    FT_GlyphSlot g = (*face)->glyph;
    
    entry->bitmap_left    = g->bitmap_left;
    entry->bitmap_top     = g->bitmap_top;
    entry->bitmapdotwidth = g->bitmap.width;
    entry->bitmapdotrows  = g->bitmap.rows;
    entry->advancedotx    = (int32_t) g->advance.x;
    entry->advancedoty    = (int32_t) g->advance.y;
    entry->vertAdvance    = (int32_t) g->metrics.vertAdvance; // !CLF: Why vertAdvance? SHould only be valid for vert layout of text.

    gGlyphCache.insert(make_pair(key, entry));

    return entry;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
  
  h = static_cast<float>(metrics->height>>6);
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

void pxFont::measureTextInternal(const char* text, uint32_t size,  float sx, float sy,
                         float& w, float& h, long& ascender, long& descender)
{
    measureTextInternal(text, size, sx, sy, w, h);
    FT_Size_Metrics* metrics = &mFace->size->metrics;
    ascender = static_cast<long>(metrics->ascender>>6);
    descender = static_cast<long>(metrics->descender>>6);
}

#ifndef PXSCENE_FONT_ATLAS
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
    float w = static_cast<float>(entry->bitmapdotwidth);
    float h = static_cast<float>(entry->bitmapdotrows);
    
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
        { texture.u1,  texture.v1 },
        { texture.u2,  texture.v1 },
        { texture.u1,  texture.v2 },
        { texture.u2,  texture.v1 },
        { texture.u1,  texture.v2 },
        { texture.u2,  texture.v2 }
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
#endif // #ifndef PXSCENE_FONT_ATLAS
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
    float w = static_cast<float>(entry->bitmapdotwidth);
    float h = static_cast<float>(entry->bitmapdotrows);
    
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
  
  h = static_cast<float>(metrics->height>>6);
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
  measure->setW(static_cast<int32_t>(w));
  measure->setH(static_cast<int32_t>(h));
  o = measure;
  
  return RT_OK; 
}

rtError pxFont::needsStyleCoercion(rtString fontStyle, bool& o)
{
    if (!fontStyle.isEmpty()) fontStyle.toLowerAscii();
    if (mFontStyle.isEmpty()) {
        rtLogWarn("Unable to detect font style of the current font. Style coercion declined.");
    }
    bool regularFont = mFontStyle.beginsWith("regular") || mFontStyle.beginsWith("normal");
    bool needsSloping = fontStyle.beginsWith("italic") || fontStyle.beginsWith("oblique");

    o = (regularFont && needsSloping);
    return RT_OK;
}

bool pxFont::coercible(const char *fontStyle) {
    rtString style = fontStyle;
    return (style.beginsWith("italic") || style.beginsWith("oblique"));
}

/**********************************************************************/
/**                    pxFontManager                                  */
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
rtRef<pxFont> pxFontManager::getFont(const char* url, const char* proxy, const rtCORSRef& cors, rtObjectRef archive, const char* fontStyle)
{
  initFT();

  rtRef<pxFont> pFont;
  uint32_t fontId;

  if (!url || !url[0])
    url = defaultFont;

  rtString key = url;
  if (pxFont::coercible(fontStyle)) {
     key = key + "+" + fontStyle;
  }

  if (false == ((key.beginsWith("http:")) || (key.beginsWith("https:"))))
  {
    pxArchive* arc = (pxArchive*)archive.getPtr();
    if (NULL != arc)
    {
      if (false == arc->isFile())
      {
        rtString data = arc->getName();
        data.append("_");
        data.append(url);
        key = data;
      }
    }
  }
  // Assign font urls an id number if they don't have one
  FontIdMap::iterator itId = mFontIdMap.find(key);
  if( itId != mFontIdMap.end()) 
  {
    fontId = itId->second;
  }
  else 
  {
    fontId = gFontId++;
    mFontIdMap.insert(make_pair(key, fontId));
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
    rtLogDebug("Create pxFont in map for %s\n",key.cString());
    pFont = new pxFont(url, fontId, proxy, fontStyle);
    pFont->setCORS(cors);
    mFontMap.insert(make_pair(fontId, pFont));
    pFont->loadResource(archive);
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

// pxTextMetrics
rtDefineObject(pxTextMetrics, rtObject);
rtDefineProperty(pxTextMetrics, height); 
rtDefineProperty(pxTextMetrics, ascent);
rtDefineProperty(pxTextMetrics, descent);
rtDefineProperty(pxTextMetrics, naturalLeading);
rtDefineProperty(pxTextMetrics, baseline);

// pxFont
rtDefineObject(pxFont, pxResource);
rtDefineProperty(pxFont, fontStyle);
rtDefineProperty(pxFont, fallbackFont);

rtDefineMethod(pxFont, getFontMetrics);
rtDefineMethod(pxFont, measureText);
rtDefineMethod(pxFont, needsStyleCoercion);

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
  if (!mTexture)
  {
    mTexture = context.createTexture(PXSCENE_FONT_ATLAS_DIM,PXSCENE_FONT_ATLAS_DIM,PXSCENE_FONT_ATLAS_DIM,PXSCENE_FONT_ATLAS_DIM, NULL);
  }
  rtLogDebug("adding a texture of [%u] size memory", w*h);
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

void pxTexturedQuads::draw(float x, float y, float* color, const pxTextEffects* pe)
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

    if (pe)
    {
        context.drawTexturedQuadsWithEffects( (int) q.verts.size()/12, &verts[0], &q.uvs[0], q.t, color, pe);
    } else {
        context.drawTexturedQuads( (int) q.verts.size()/12, &verts[0], &q.uvs[0], q.t, color);
    }
  }
}

void pxTexturedQuads::draw(float x, float y)
{
    draw(x, y, mColor, mpTextEffects);
}

void pxTexturedQuads::setColor(uint32_t c)
{
    mColor[0]/*R*/ = (float)((c>>24) & 0xff) / 255.0f;
    mColor[1]/*G*/ = (float)((c>>16) & 0xff) / 255.0f;
    mColor[2]/*B*/ = (float)((c>> 8) & 0xff) / 255.0f;
    mColor[3]/*A*/ = (float)((c>> 0) & 0xff) / 255.0f;
}
void pxTexturedQuads::setTextEffects(const pxTextEffects* pe)
{
    mpTextEffects = pe;
}

#endif
