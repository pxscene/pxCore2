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

// pxFont.h

#ifndef PX_FONT_H
#define PX_FONT_H

#include "rtString.h"
#include "rtRef.h"
#include "rtCORS.h"

// TODO it would be nice to push this back into implemention
#include <ft2build.h>
#include FT_FREETYPE_H

#include "pxScene2d.h"
#include <map>
#include <vector>

using std::vector;

class pxText;
class pxFont;

#define defaultPixelSize 16
#define defaultFont "FreeSans.ttf"

class rtFileDownloadRequest;

#if defined WIN32
#include<inttypes.h>
typedef uint32_t u_int32_t;
#endif

struct GlyphCacheEntry
{
  int32_t bitmap_left;
  int32_t bitmap_top;
  int32_t bitmapdotwidth;
  int32_t bitmapdotrows;
  int32_t advancedotx;
  int32_t advancedoty;
  int32_t vertAdvance;
};

struct GlyphTextureEntry
{
  pxTextureRef t;
  float u1, v1, u2, v2;
  GlyphTextureEntry(): u1(0),v1(0),u2(0),v2(0){}
};

#ifdef PXSCENE_FONT_ATLAS
class pxFontAtlas
{
public:


  struct row
  {
    uint32_t top;
    uint32_t height;
    uint32_t rFence;
  };


  pxFontAtlas();

  bool addGlyph(uint32_t w, uint32_t h, void* buffer, GlyphTextureEntry& e);
  void clearTexture();


  private:

  uint32_t fence;
  pxTextureRef mTexture;
  vector<row> mRows;
};

class pxTexturedQuads
{
  // limit the size of vectors per quad to prevent memory
  // issues when rendering
  static const int maxVectorSize = 60000;
  public:

  struct quads
  {
    vector<float> verts;
    vector<float> uvs;
    pxTextureRef t;
  };

  pxTexturedQuads() {}

  void addQuad(float x1,float y1,float x2,float y2, float u1, float v1, float u2, float v2, pxTextureRef t)
  {
    if (mQuads.empty() || mQuads[mQuads.size()-1].t != t || mQuads[mQuads.size()-1].verts.size() >= maxVectorSize)
    {
      quads q;
      q.t = t;
      mQuads.push_back(q);
    }

    quads& q = mQuads[mQuads.size()-1];
    vector<float>& v = q.verts;
    vector<float>& u = q.uvs;

    // triangle 1
    v.push_back(x1);
    v.push_back(y1);
    v.push_back(x2);
    v.push_back(y1);
    v.push_back(x1);
    v.push_back(y2);
    // triangle 2
    v.push_back(x2);
    v.push_back(y1);
    v.push_back(x1);
    v.push_back(y2);
    v.push_back(x2);
    v.push_back(y2);

    // triangle 1 uvs
    u.push_back(u1);
    u.push_back(v1);
    u.push_back(u2);
    u.push_back(v1);
    u.push_back(u1);
    u.push_back(v2);
    // triangle 2 uvs
    u.push_back(u2);
    u.push_back(v1);
    u.push_back(u1);
    u.push_back(v2);
    u.push_back(u2);
    u.push_back(v2);
  }

  void draw(float x, float y, float* color);

  void clear()
  {
    mQuads.clear();
  }

private:
  vector<quads> mQuads;
};

#endif

/**********************************************************************
 * 
 * pxTextMetrics
 * 
 **********************************************************************/
class pxTextMetrics: public rtObject 
{

public:
	pxTextMetrics():mHeight(0),mAscent(0),mDescent(0),mNaturalLeading(0),mBaseline(0) {}
	virtual ~pxTextMetrics() {}

	rtDeclareObject(pxTextMetrics, rtObject);
	rtReadOnlyProperty(height, height, float); 
	rtReadOnlyProperty(ascent, ascent, float);
	rtReadOnlyProperty(descent, descent, float);
  rtReadOnlyProperty(naturalLeading, naturalLeading, float);
  rtReadOnlyProperty(baseline, baseline, float);
 
	float height()             const { return mHeight; }
	rtError height(float& v)   const { v = mHeight; return RT_OK;   }
	rtError setHeight(float v)       { mHeight = v; return RT_OK;   }

	float ascent()             const { return mAscent; }
	rtError ascent(float& v)   const { v = mAscent; return RT_OK;   }
	rtError setAscent(float v)       { mAscent = v; return RT_OK;   } 

	float descent()             const { return mDescent; }
	rtError descent(float& v)   const { v = mDescent; return RT_OK;   }
	rtError setDescent(float v)       { mDescent = v; return RT_OK;   } 
 
 	float naturalLeading()             const { return mNaturalLeading; }
	rtError naturalLeading(float& v)   const { v = mNaturalLeading; return RT_OK;   }
	rtError setNaturalLeading(float v)       { mNaturalLeading = v; return RT_OK;   } 
  
 	float baseline()             const { return mBaseline; }
	rtError baseline(float& v)   const { v = mBaseline; return RT_OK;   }
	rtError setBaseline(float v)       { mBaseline = v; return RT_OK;   }   
   
  private:
   	float mHeight;
    float mAscent;
    float mDescent;
    float mNaturalLeading;
    float mBaseline;
};

/**********************************************************************
 * 
 * pxTextMeasurements
 * 
 **********************************************************************/
class pxTextSimpleMeasurements: public rtObject 
{
public:
	pxTextSimpleMeasurements():mw(0),mh(0) {}
	virtual ~pxTextSimpleMeasurements() {}

	rtDeclareObject(pxTextSimpleMeasurements, rtObject);
  rtReadOnlyProperty(w, w, int32_t);
  rtReadOnlyProperty(h, h, int32_t);  

  int32_t w() const { return mw;  }
  rtError w(int32_t& v) const { v = mw;  return RT_OK; }
  int32_t h() const { return mh; }
  rtError h(int32_t& v) const { v = mh; return RT_OK; }   

  void setW(int32_t v) { mw = v; }
  void setH(int32_t v) { mh = v; }  
    
protected:
  int32_t mw;
  int32_t mh;
};

/**********************************************************************
 * 
 * pxFont
 * 
 **********************************************************************/
class pxFont: public pxResource {

public:
	pxFont(rtString fontUrl, uint32_t id, rtString proxyUrl);
	virtual ~pxFont() ;

	rtDeclareObject(pxFont, pxResource);
  rtReadOnlyProperty(ready, ready, rtObjectRef);
  
  rtMethod1ArgAndReturn("getFontMetrics", getFontMetrics, uint32_t, rtObjectRef);
  rtError getFontMetrics(uint32_t pixelSize, rtObjectRef& o);
  rtMethod2ArgAndReturn("measureText", measureText, uint32_t, rtString, rtObjectRef);
  rtError measureText(uint32_t, rtString, rtObjectRef& o);   
    
  // FT Face related functions
  void setPixelSize(uint32_t s);  
  const GlyphCacheEntry* getGlyph(uint32_t codePoint);
  GlyphTextureEntry getGlyphTexture(uint32_t codePoint, float sx, float sy);  
  void getMetrics(uint32_t size, float& height, float& ascender, float& descender, float& naturalLeading);
  void getHeight(uint32_t size, float& height);
  void measureText(const char* text, uint32_t size, float& w, float& h);
  void measureTextInternal(const char* text, uint32_t size,  float sx, float sy, 
                   float& w, float& h);
  void measureTextChar(u_int32_t codePoint, uint32_t size,  float sx, float sy, 
                         float& w, float& h);
  #ifndef PXSCENE_FONT_ATLAS
  void renderText(const char *text, uint32_t size, float x, float y, 
                  float sx, float sy, 
                  float* color, float mw);
  #endif

  #ifdef PXSCENE_FONT_ATLAS
  // Should reinvoke on changes to text, size, or scale params
  void renderTextToQuads(const char *text, uint32_t size, 
                        float nsx, float nsy, 
                        pxTexturedQuads& quads,
                        float x = 0, float y = 0);
  #endif
  virtual void init() {}
  bool isFontLoaded() { return mInitialized;}

	void setFontData(const FT_Byte*  fontData, FT_Long size, const char* n);
	virtual void setupResource();
  void clearDownloadedData();
  uint32_t getFontId() { return mFontId;}
   
protected:
  // Implementation for pxResource virtuals
  virtual uint32_t loadResourceData(rtFileDownloadRequest* fileDownloadRequest);
  
private:
  void loadResourceFromFile();
  void loadResourceFromArchive(rtObjectRef archiveRef);
  rtError init(const char* n);
  rtError init(const FT_Byte*  fontData, FT_Long size, const char* n); 

  // FreeType font info
  uint32_t mFontId;
  FT_Face mFace;
  uint32_t mPixelSize;
  char* mFontData; // for remote fonts loaded into memory
  size_t mFontDataSize;
  rtMutex mFontMutex;
	rtMutex mFontDataMutex;
	char* mFontDownloadedData;
	size_t mFontDownloadedDataSize;
	rtString mFontDataUrl;

};

// Weak Map
typedef std::map<uint32_t, pxFont*> FontMap;
typedef std::map<rtString, uint32_t> FontIdMap;
class pxFontManager
{
  
  public: 
    
    static rtRef<pxFont> getFont(const char* url, const char* proxy = NULL, const rtCORSRef& cors = NULL, rtObjectRef archive = NULL);
    static void removeFont(uint32_t fontId);
    static void clearAllFonts();
    
  protected: 
    static void initFT();  
    static FontMap mFontMap;
    static FontIdMap mFontIdMap;
    static bool init;
    
};
#endif
