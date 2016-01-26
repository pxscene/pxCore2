// pxCore CopyRight 2007-2015 John Robinson
// pxFont.h

#ifndef PX_FONT_H
#define PX_FONT_H

// TODO it would be nice to push this back into implemention
#include <ft2build.h>
#include FT_FREETYPE_H

#include "rtString.h"
#include "rtRefT.h"
#include "pxScene2d.h"
#include <map>

class pxText;
class pxFont;

#define defaultPixelSize 16
#define defaultFont "FreeSans.ttf"


class pxFileDownloadRequest;

struct GlyphCacheEntry
{
  int bitmap_left;
  int bitmap_top;
  int bitmapdotwidth;
  int bitmapdotrows;
  int advancedotx;
  int advancedoty;
  int vertAdvance;

  pxTextureRef mTexture;
};



/**********************************************************************
 * 
 * pxTextMetrics
 * 
 **********************************************************************/
class pxTextMetrics: public rtObject {

public:
	pxTextMetrics(){  }
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
class pxTextSimpleMeasurements: public rtObject {

public:
	pxTextSimpleMeasurements() { 

  }
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
class pxFont: public rtResource {

public:
	pxFont(rtString fontUrl);
	virtual ~pxFont() ;

	rtDeclareObject(pxFont, rtResource);
  rtReadOnlyProperty(ready, ready, rtObjectRef);
  
  rtMethod1ArgAndReturn("getFontMetrics", getFontMetrics, uint32_t, rtObjectRef);
  rtError getFontMetrics(uint32_t pixelSize, rtObjectRef& o);
  rtMethod2ArgAndReturn("measureText", measureText, uint32_t, rtString, rtObjectRef);
  rtError measureText(uint32_t, rtString, rtObjectRef& o);   
    
  // FT Face related functions
  void setPixelSize(uint32_t s);  
  const GlyphCacheEntry* getGlyph(uint32_t codePoint);  
  void getMetrics(uint32_t size, float& height, float& ascender, float& descender, float& naturalLeading);
  void getHeight(uint32_t size, float& height);
  void measureText(const char* text, uint32_t size, float& w, float& h);
  void measureTextInternal(const char* text, uint32_t size,  float sx, float sy, 
                   float& w, float& h);
  void measureTextChar(u_int32_t codePoint, uint32_t size,  float sx, float sy, 
                         float& w, float& h);                   
  void renderText(const char *text, uint32_t size, float x, float y, 
                  float sx, float sy, 
                  float* color, float mw);

  virtual void init() {}
  bool isFontLoaded() { return mInitialized;}
   
protected:
  // Implementation for rtResource virtuals
  virtual bool loadResourceData(pxFileDownloadRequest* fileDownloadRequest);
  
private:
  void loadResourceFromFile();
  rtError init(const char* n);
  rtError init(const FT_Byte*  fontData, FT_Long size, const char* n); 

  // FreeType font info
  uint32_t mFontId;
  FT_Face mFace;
  uint32_t mPixelSize;
  char* mFontData; // for remote fonts loaded into memory

};

// Weak Map
typedef map<rtString, pxFont*> FontMap;

class pxFontManager
{
  
  public: 
    
    static rtRefT<pxFont> getFont(const char* url);
    static void removeFont(rtString fontName);
    
  protected: 
    static void initFT();  
    static FontMap mFontMap;
    static bool init;
    
};
#endif

