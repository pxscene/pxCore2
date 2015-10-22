// pxCore CopyRight 2007-2015 John Robinson
// pxText.h

#ifndef PX_TEXT_H
#define PX_TEXT_H

// TODO it would be nice to push this back into implemention
#include <ft2build.h>
#include FT_FREETYPE_H

#include "rtString.h"
#include "rtRefT.h"
#include "pxScene2d.h"

#define defaultPixelSize 16
#define defaultFace "FreeSans.ttf"
class pxText;

class pxFace;
typedef rtRefT<pxFace> pxFaceRef;

class pxFileDownloadRequest;
typedef struct _FontDownloadRequest
{
  pxFileDownloadRequest* fileDownloadRequest;
} FontDownloadRequest;

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

class pxFace
{
public:
  pxFace();
  virtual ~pxFace();
  
  rtError init(const char* n);
  rtError init(const FT_Byte*  fontData, FT_Long size, const char* n);
  
  void setFaceName(const char* n);
  bool isInitialized() { return mInitialized; }
  void onDownloadComplete(const FT_Byte* fontData, FT_Long size, const char* n);
  void addListener(pxText* pText);

  virtual unsigned long AddRef() 
  {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() 
  {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }
    
  void setPixelSize(uint32_t s);  
  const GlyphCacheEntry* getGlyph(uint32_t codePoint);  
  void getMetrics(uint32_t size, float& height, float& ascender, float& descender, float& naturalLeading);
  void getHeight(uint32_t size, float& height);
  void measureText(const char* text, uint32_t size,  float sx, float sy, 
                   float& w, float& h);
  void measureTextChar(u_int32_t codePoint, uint32_t size,  float sx, float sy, 
                         float& w, float& h);                   
  void renderText(const char *text, uint32_t size, float x, float y, 
                  float sx, float sy, 
                  float* color, float mw);

private:
  uint32_t mFaceId;
  rtString mFaceName;
  FT_Face mFace;
  uint32_t mPixelSize;
  rtAtomic mRefCount;
  
  char* mFontData; // for remote fonts loaded into memory
  
  bool mInitialized;
  vector<pxText*> mListeners;
};

class pxText: public pxObject 
{
public:
  rtDeclareObject(pxText, pxObject);
  rtProperty(text, text, setText, rtString);
  rtProperty(textColor, textColor, setTextColor, uint32_t);
  rtProperty(pixelSize, pixelSize, setPixelSize, uint32_t);
  rtProperty(faceURL, faceURL, setFaceURL, rtString);

  pxText(pxScene2d* scene);
  ~pxText();
  rtError text(rtString& s) const;
  virtual rtError setText(const char* text);

  rtError textColor(uint32_t& /*c*/) const {
    
    return RT_OK;
  }

  rtError setTextColor(uint32_t c) {
    mTextColor[0] = (float)((c>>24)&0xff)/255.0f;
    mTextColor[1] = (float)((c>>16)&0xff)/255.0f;
    mTextColor[2] = (float)((c>>8)&0xff)/255.0f;
    mTextColor[3] = (float)((c>>0)&0xff)/255.0f;
    return RT_OK;
  }

  rtError faceURL(rtString& v) const { v = mFaceURL; return RT_OK; }
  virtual rtError setFaceURL(const char* s);

  rtError pixelSize(uint32_t& v) const { v = mPixelSize; return RT_OK; }
  virtual rtError setPixelSize(uint32_t v);

  virtual void update(double t);

  virtual rtError Set(const char* name, const rtValue* value)
  {
#if 1
    mDirty = mDirty || (!strcmp(name,"w") ||
              !strcmp(name,"h") ||
              !strcmp(name,"text") ||
              !strcmp(name,"pixelSize") ||
              !strcmp(name,"faceURL") ||
              !strcmp(name,"textColor"));
#else
    mDirty = true;
#endif
    rtError e = pxObject::Set(name, value);
    

    return e;
  }

  void onFontDownloadComplete(FontDownloadRequest fontDownloadRequest);
  static void checkForCompletedDownloads(int maxTimeInMilliseconds=10);
  virtual void fontLoaded();

 protected:
  virtual void draw();
  rtString mText;
// TODO should we just use a face object instead of urls
  bool mFontLoaded;
  rtString mFaceURL;
  pxFaceRef mFace;
  float mTextColor[4];
  uint32_t mPixelSize;
  bool mDirty;
  pxContextFramebufferRef mCached;
  pxFileDownloadRequest* mFontDownloadRequest;
  
  virtual float getFBOWidth() { return mw; }
  virtual float getFBOHeight() { return mh; }
};

#endif
