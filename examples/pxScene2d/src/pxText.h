// pxCore CopyRight 2007-2015 John Robinson
// pxText.h

#ifndef PX_TEXT_H
#define PX_TEXT_H

#include "rtString.h"
#include "rtRefT.h"
#include "pxScene2d.h"

class pxText: public pxObject {
public:
  rtDeclareObject(pxText, pxObject);
  rtProperty(text, text, setText, rtString);
  rtProperty(textColor, textColor, setTextColor, uint32_t);
  rtProperty(pixelSize, pixelSize, setPixelSize, uint32_t);

  pxText();
  rtError text(rtString& s) const;
  rtError setText(const char* text);

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

  rtError pixelSize(uint32_t& v) const { v = mPixelSize; return RT_OK; }
  rtError setPixelSize(uint32_t v);

 protected:
  virtual void draw();
  rtString mText;
  float mTextColor[4];
  uint32_t mPixelSize;
};

#endif
