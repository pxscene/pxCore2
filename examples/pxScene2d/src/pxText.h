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
#include "pxFont.h"

class pxText;







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
  virtual void onInit();
  
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

  virtual void fontLoaded(const char * value);

 protected:
  virtual void draw();
  rtString mText;
// TODO should we just use a face object instead of urls
  bool mFontLoaded;
  rtString mFaceURL;

pxFont* mFont;
  float mTextColor[4];
  uint32_t mPixelSize;
  bool mDirty;
  pxContextFramebufferRef mCached;
  pxFileDownloadRequest* mFontDownloadRequest;
  
  virtual float getFBOWidth() { return mw; }
  virtual float getFBOHeight() { return mh; }
};

#endif
