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

  pxText(pxScene2d* scene);
  virtual ~pxText();

  rtError text(rtString& s) const;
  virtual rtError setText(const char* text);
  rtError removeResourceListener();

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
  
  virtual void onInit();
  
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
              !strcmp(name,"sy");
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