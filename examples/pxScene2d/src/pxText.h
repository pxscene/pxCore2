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

  pxText(pxScene2d* scene);
  virtual ~pxText();
  rtError text(rtString& s) const;
  virtual rtError setText(const char* text);
  rtError removeResourceListener();

  rtError textColorInternal(uint32_t& c) const
  {
#ifdef PX_LITTLEENDIAN_PIXELS

    c = ((uint8_t) (mTextColor[0] * 255.0f) << 24) |  // R
        ((uint8_t) (mTextColor[1] * 255.0f) << 16) |  // G
        ((uint8_t) (mTextColor[2] * 255.0f) <<  8) |  // B
        ((uint8_t) (mTextColor[3] * 255.0f) <<  0);   // A
#else

    c = ((uint8_t) (mTextColor[3] * 255.0f) << 24) |  // A
        ((uint8_t) (mTextColor[2] * 255.0f) << 16) |  // B
        ((uint8_t) (mTextColor[1] * 255.0f) <<  8) |  // G
        ((uint8_t) (mTextColor[0] * 255.0f) <<  0);   // R
#endif

    
    return RT_OK;
  }

  rtError setTextColorInternal(uint32_t c)
  {
    mTextColor[PX_RED  ] = (float)((c>>24) & 0xff) / 255.0f;
    mTextColor[PX_GREEN] = (float)((c>>16) & 0xff) / 255.0f;
    mTextColor[PX_BLUE ] = (float)((c>> 8) & 0xff) / 255.0f;
    mTextColor[PX_ALPHA] = (float)((c>> 0) & 0xff) / 255.0f;
    return RT_OK;
  }
  
  rtError textColor(rtValue &c) const
  {
    uint32_t cc = 0;
    rtError err = textColorInternal(cc);
    
    c = cc;
    
    return err;
  }
  
  rtError setTextColor(rtValue c)
  {
    // Set via STRING...
    if( c.getType() == 's')
    {
      rtString str = c.toString();
      
      uint8_t r,g,b, a;
      if( web2rgb( str, r, g, b, a) == RT_OK)
      {
        mTextColor[PX_RED  ] = (float) r / 255.0f;  // R
        mTextColor[PX_GREEN] = (float) g / 255.0f;  // G
        mTextColor[PX_BLUE ] = (float) b / 255.0f;  // B
        mTextColor[PX_ALPHA] = (float) a / 255.0f;  // A
        
        return RT_OK;
      }
      
      return RT_FAIL;
    }
    
    // Set via UINT32...
    uint32_t clr = c.toUInt32();
    
    return setTextColorInternal(clr);
  }

  rtError fontUrl(rtString& v) const { getFontResource()->url(v); return RT_OK; }
  virtual rtError setFontUrl(const char* s);

  rtError pixelSize(uint32_t& v) const { v = mPixelSize; return RT_OK; }
  virtual rtError setPixelSize(uint32_t v);
  
  rtError font(rtObjectRef& o) const { o = mFont; return RT_OK; }
  virtual rtError setFont(rtObjectRef o);
  
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
};

#endif // PX_TEXT_H