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

// pxText.h

#ifndef PX_TEXT_H
#define PX_TEXT_H

// TODO it would be nice to push this back into implemention
#include <ft2build.h>
#include FT_FREETYPE_H

#include "rtString.h"
#include "rtRef.h"
#include "pxScene2d.h"
#include "pxFont.h"
#include "pxResource.h"

#include "pxColorNames.h"

class pxText: public pxObject, pxResourceListener 
{
public:
  rtDeclareObject(pxText, pxObject);
  rtProperty(text, text, setText, rtString);
  rtProperty(textColor, textColor, setTextColor, rtValue);
  rtProperty(pixelSize, pixelSize, setPixelSize, uint32_t);
  rtProperty(fontUrl, fontUrl, setFontUrl, rtString);  
  rtProperty(font, font, setFont, rtObjectRef);
  rtMethodNoArgAndReturn("texture", texture, uint32_t);

  rtProperty(shadow,        shadow,        setShadow,        bool);
  rtProperty(shadowColor,   shadowColor,   setShadowColor,   rtValue);
  rtProperty(shadowOffsetX, shadowOffsetX, setShadowOffsetX, float);
  rtProperty(shadowOffsetY, shadowOffsetY, setShadowOffsetY, float);
  rtProperty(shadowBlur,    shadowBlur,    setShadowBlur,    float);

  rtProperty(highlight,             highlight,             setHighlight,              bool);
  rtProperty(highlightColor,        highlightColor,        setHighlightColor,         rtValue);
  rtProperty(highlightOffset,       highlightOffset,       setHighlightOffset,        float);
  rtProperty(highlightPaddingLeft,  highlightPaddingLeft,  setHighlightPadddingLeft,  float);
  rtProperty(highlightPaddingRight, highlightPaddingRight, setHighlightPadddingRight, float);


  pxText(pxScene2d* scene);
  virtual ~pxText();

  rtError text(rtString& s) const;
  virtual rtError setText(const char* text);

  rtError setColor(float *var, rtValue c);
  rtError colorFloat4_to_UInt32( const float *clr, uint32_t& c) const;
  rtError colorUInt32_to_Float4(float *clr, uint32_t c) const;

  rtError textColor(rtValue &c) const;
  rtError setTextColor(rtValue c);

  rtError fontUrl(rtString& v) const { getFontResource()->url(v); return RT_OK; }
  virtual rtError setFontUrl(const char* s);

  rtError pixelSize(uint32_t& v) const { v = mPixelSize; return RT_OK; }
  virtual rtError setPixelSize(uint32_t v);

  rtError font(rtObjectRef& o) const { o = mFont; return RT_OK; }
  virtual rtError setFont(rtObjectRef o);

  virtual rtError texture(uint32_t & v);

  // - - - - - - - - - - - - - - - - - - Shadow - - - - - - - - - - - - - - - - - -

  rtError shadow(bool &v) const                      { v = mShadow;  return RT_OK; }
  virtual rtError setShadow(bool v)                  { mShadow = v;  return RT_OK; }

  rtError shadowColor(rtValue &c) const;
  rtError setShadowColor(rtValue c);

  rtError shadowOffsetX(float &v) const              { v = mShadowOffsetX; return RT_OK; }
  virtual rtError setShadowOffsetX(float v)          { mShadowOffsetX = v; return RT_OK; }

  rtError shadowOffsetY(float &v) const              { v = mShadowOffsetY; return RT_OK; }
  virtual rtError setShadowOffsetY(float v)          { mShadowOffsetY = v; return RT_OK; }

  rtError shadowBlur(float &v) const                 { v = mShadowBlur;    return RT_OK; }
  virtual rtError setShadowBlur(float v)             { mShadowBlur = v;    return RT_OK; }

  // - - - - - - - - - - - - - - - - - - Highlight - - - - - - - - - - - - - - - - - -

  rtError highlight(bool &v) const                   { v = mHighlight;  return RT_OK; }
  virtual rtError setHighlight(bool v)               { mHighlight = v;  return RT_OK; }

  rtError highlightColor(rtValue &c) const;
  rtError setHighlightColor(rtValue c);

  rtError highlightOffset(float &v) const            { v = mHighlightOffset;       return RT_OK; }
  virtual rtError setHighlightOffset(float v)        { mHighlightOffset = v;       return RT_OK; }

  rtError highlightPaddingLeft(float &v) const       { v = mHighlightPaddingLeft;  return RT_OK; }
  virtual rtError setHighlightPadddingLeft(float v)  { mHighlightPaddingLeft = v;  return RT_OK; }

  rtError highlightPaddingRight(float &v) const      { v = mHighlightPaddingRight; return RT_OK; }
  virtual rtError setHighlightPadddingRight(float v) { mHighlightPaddingRight = v; return RT_OK; }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  virtual void onInit();

  rtError removeResourceListener();
  
  virtual rtError Set(uint32_t i, const rtValue* value) override
  {
    (void)i;
    (void)value;
    rtLogError("pxText::Set(uint32_t, const rtValue*) - not implemented");
    return RT_ERROR_NOT_IMPLEMENTED;
  }

  virtual rtError Set(const char* name, const rtValue* value) override
  {
    //rtLogInfo("pxText::Set %s\n",name);
#if 1
    mDirty = mDirty ||
              !strcmp(name,"text") ||
              !strcmp(name,"pixelSize") ||
              !strcmp(name,"fontUrl") ||
              !strcmp(name,"font") ||
              !strcmp(name,"sx") || 
              !strcmp(name,"sy")                   ||

              !strcmp(name,"shadow")               ||
              !strcmp(name,"shadowColor")          ||
              !strcmp(name,"shadowOffsetX")        ||
              !strcmp(name,"shadowOffsetY")        ||
              !strcmp(name,"shadowBlur")           ||

              !strcmp(name,"highlight")            ||
              !strcmp(name,"highlightColor")       ||
              !strcmp(name,"highlightOffset")      ||
              !strcmp(name,"highlightPaddingLeft") ||
              !strcmp(name,"highlightPaddingRight");
#else
    mDirty = true;
#endif
    rtError e = pxObject::Set(name, value);
    

    return e;
  }

  virtual void resourceReady(rtString readyResolution);
  virtual void resourceDirty();
  virtual void sendPromise();
  virtual float getOnscreenWidth();
  virtual float getOnscreenHeight();
  virtual void createNewPromise();
  virtual void dispose(bool pumpJavascript);
  virtual uint64_t textureMemoryUsage(std::vector<rtObject*> &objectsCounted);
  
 protected:
  virtual void draw();
  // !CLF ToDo: Could mFont.send(...) be used in places where mFont is needed, instead
  // of this getFontResource?
  inline pxFont* getFontResource() const { return (pxFont*)mFont.getPtr(); }  
  
  rtString mText;
// TODO should we just use a font object instead of Urls
  bool mFontLoaded;
  bool mFontFailed;
//  rtString mFontUrl;

  rtObjectRef mFont;
  
  float mTextColor[4];
  uint32_t mPixelSize;

  bool     mShadow;
  float    mShadowColor[4];
  float    mShadowOffsetX;
  float    mShadowOffsetY;
  float    mShadowBlur;

  bool     mHighlight;
  float    mHighlightColor[4];
  float    mHighlightOffset;
  float    mHighlightPaddingLeft;
  float    mHighlightPaddingRight;

  bool mDirty;
  pxContextFramebufferRef mCached;
  rtFileDownloadRequest* mFontDownloadRequest;
  
  virtual float getFBOWidth();
  virtual float getFBOHeight();
  bool mListenerAdded;

  #ifdef PXSCENE_FONT_ATLAS
  pxTexturedQuads mQuads;
  #endif
  pxContextFramebufferRef mTextFbo;
};

#endif // PX_TEXT_H