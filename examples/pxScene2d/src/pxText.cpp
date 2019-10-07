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

// pxText.cpp

#include "pxText.h"
#include "rtFileDownloader.h"
#include "pxTimer.h"
#include "pxFont.h"
#include "pxContext.h"
#include <algorithm>

extern pxContext context;

pxText::pxText(pxScene2d* scene):pxObject(scene), mFontLoaded(false), mFontFailed(false), mDirty(true), mFontDownloadRequest(NULL), mListenerAdded(false),
                                 mTextFbo()

{
  float c[4] = {1, 1, 1, 1}; // WHITE
  memcpy(mTextColor, c, sizeof(mTextColor));
  // Default to use default font
  mFont = pxFontManager::getFont(defaultFont, NULL, NULL, scene->getArchive(), NULL);
  mPixelSize = defaultPixelSize;
}

pxText::~pxText()
{
  removeResourceListener();
  if (mTextFbo.getPtr() != NULL)
  {
    mTextFbo->resetFbo();
  }
  mTextFbo = NULL;
}

void pxText::onInit()
{
  pxObject::onInit();
  mInitialized = true;

  if( getFontResource() != NULL && getFontResource()->isFontLoaded()) {
    resourceReady("resolve");
  }
}
rtError pxText::text(rtString& s) const { s = mText; return RT_OK; }

void pxText::sendPromise() 
{
  if(mInitialized && mFontLoaded && !((rtPromise*)mReady.getPtr())->status())
  {
    //rtLogDebug("pxText SENDPROMISE\n");
    mReady.send("resolve",this); 
  }
}

rtError pxText::setText(const char* s) 
{ 
  //rtLogInfo("pxText::setText\n");
  if( !mText.compare(s)){
    rtLogDebug("pxText.setText setting to same value %s and %s\n", mText.cString(), s);
    return RT_OK;
  }
  mText = s; 
  if( getFontResource() != NULL && getFontResource()->isFontLoaded())
  {
    getFontResource()->measureTextInternal(s, mPixelSize, 1.0, 1.0, mw, mh);
  }
  return RT_OK; 
}

rtError pxText::setPixelSize(uint32_t v) 
{   
  //rtLogInfo("pxText::setPixelSize\n");
  mPixelSize = v; 
  if( getFontResource() != NULL && getFontResource()->isFontLoaded())
  {
    getFontResource()->measureTextInternal(mText, mPixelSize, 1.0, 1.0, mw, mh);
  }
  return RT_OK; 
}

void pxText::resourceReady(rtString readyResolution)
{
  if( !readyResolution.compare("resolve"))
  {    
    mFontLoaded=true;

    // pxText gets its height and width from the text itself, 
    // so measure it
    if (getFontResource() != NULL) {
       getFontResource()->measureTextInternal(mText, mPixelSize, 1.0, 1.0, mw, mh);
	  }
	
    mDirty=true;  
    mScene->mDirty = true;
    // !CLF: ToDo Use pxObject::onTextureReady() and rename it.
    if( mInitialized) 
    {
      if( !mParent)
      {
        // Send the promise here because the text will not get an
        // update call until it has parent
        sendPromise();
      }
      pxObject::onTextureReady();
    }
  }
  else 
  {
      mFontFailed = true;
      pxObject::onTextureReady();
      mReady.send("reject",this);
  }     
}

void pxText::resourceDirty()
{
  pxObject::onTextureReady();
}

void pxText::draw() 
{
  static pxTextureRef nullMaskRef;
  if( getFontResource() != NULL && getFontResource()->isFontLoaded())
  {
    pxContextFramebufferRef previousSurface;
    pxContextFramebufferRef cached;

    if ((msx < 1.0) || (msy < 1.0))
    {
      context.pushState();
      previousSurface = context.getCurrentFramebuffer();
      cached = context.createFramebuffer(getFBOWidth(),getFBOHeight());

      if (cached.getPtr())
      {
        if (context.setFramebuffer(cached) == PX_OK)
        {
          pxMatrix4f m;
          context.setMatrix(m);
          context.setAlpha(1.0);
          context.clear(getFBOWidth(), getFBOHeight());
        }
      }
    }// ENDIF - Scale Down

#ifdef PXSCENE_FONT_ATLAS
    if (mDirty)
    {
      getFontResource()->renderTextToQuads(mText,mPixelSize,msx,msy,mQuads);
      mDirty = false;
    }

    // DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW
    mQuads.draw(0, 0, mTextColor);// DRAW DRAW
    // DRAW DRAW DRAW DRAW DRAW DRAW DRAW DRAW

#else
    if (getFontResource() != NULL)
    {
      getFontResource()->renderText(mText, mPixelSize, 0, 0, msx, msy, mTextColor, mw);
    }
#endif
    if ((msx < 1.0) || (msy < 1.0))
    {
      context.setFramebuffer(previousSurface);
      context.popState();
      if (cached.getPtr() && cached->getTexture().getPtr())
      {
        context.drawImage(0, 0, (mw > MAX_TEXTURE_WIDTH  ? MAX_TEXTURE_WIDTH  : mw),
                                (mh > MAX_TEXTURE_HEIGHT ? MAX_TEXTURE_HEIGHT : mh),
                          cached->getTexture(), nullMaskRef);
      }
    }
  }
}

rtError pxText::setFontUrl(const char* s)
{
  //rtLogInfo("pxText::setFaceUrl for %s\n",s);
  if (!s || !s[0]) {
    s = defaultFont;
  }

  mFontLoaded = false;
  mFontFailed = false;
  createNewPromise();

  removeResourceListener();
  mFont = pxFontManager::getFont(s, NULL, NULL, mScene->getArchive(), NULL);
  mListenerAdded = true;

  if (getFontResource() != NULL)
  {
    getFontResource()->addListener(this);
  }
  
  return RT_OK;
}

rtError pxText::setFont(rtObjectRef o)
{
  mFont = NULL;
  mFontLoaded = false;
  mFontFailed = false;

  createNewPromise();
  removeResourceListener();

  if (o){
    rtString desc;
    rtError err = o.sendReturns<rtString>("description", desc);
    if (err == RT_OK && desc.compare("pxFont") == 0) {
        mFont = o;
     }
   }

  if(getFontResource() == NULL) {
    resourceReady("reject");
  }

  mListenerAdded = true;
  if (getFontResource() != NULL) {
    getFontResource()->addListener(this);
  }
  return RT_OK;
}

rtError pxText::texture(uint32_t &v)
{
  v = 0;
  if (mTextFbo.getPtr() != NULL && mTextFbo->getTexture() != NULL && !mDirty)
  {
    v = mTextFbo->getTexture()->getNativeId();
  }
  else
  {
    pxContextFramebufferRef previousSurface;
    context.pushState();
    previousSurface = context.getCurrentFramebuffer();
    mTextFbo = context.createFramebuffer(getFBOWidth(), getFBOHeight());
    if (mTextFbo.getPtr())
    {
      mTextFbo->enableFlipRendering(true);
      if (context.setFramebuffer(mTextFbo) == PX_OK)
      {
        pxMatrix4f m;
        context.setMatrix(m);
        context.setAlpha(1.0);
        context.clear(getFBOWidth(), getFBOHeight());
        bool previousDirty = mDirty;
        draw();
        mDirty = previousDirty;
      }
      context.setFramebuffer(previousSurface);
      v = mTextFbo->getTexture()->getNativeId();
    }
    context.popState();
  }
  return RT_OK;
}

float pxText::getOnscreenWidth()
{
  // TODO review max texture handling
  return (mw > MAX_TEXTURE_WIDTH?MAX_TEXTURE_WIDTH:mw);
}
float pxText::getOnscreenHeight()
{
  // TODO review max texture handling
  return (mh > MAX_TEXTURE_HEIGHT?MAX_TEXTURE_HEIGHT:mh);
}

float pxText::getFBOWidth()
{ 
  if( mw > MAX_TEXTURE_WIDTH) 
  {
    rtLogWarn("Text width is larger than maximum texture allowed: %lf.  Maximum texture size of %d will be used.",mw, MAX_TEXTURE_WIDTH);  
    return MAX_TEXTURE_WIDTH;
  }
  else 
    return mw; 
}

float pxText::getFBOHeight()
{ 
  if( mh > MAX_TEXTURE_HEIGHT) 
  {
    rtLogWarn("Text height is larger than maximum texture allowed: %lf.  Maximum texture size of %d will be used.",mh, MAX_TEXTURE_HEIGHT);
    return MAX_TEXTURE_HEIGHT;
  }
  else 
    return mh; 
}

rtError pxText::removeResourceListener()
{
  if (mListenerAdded)
  {
    if (getFontResource())
    {
      getFontResource()->removeListener(this);
    }
    mListenerAdded = false;
  }
  return RT_OK;
}

void pxText::createNewPromise()
{
  // Only create a new promise if the existing one has been
  // resolved or rejected already and font did not fail
  if(!mFontFailed && ((rtPromise*)mReady.getPtr())->status())
  {
    rtLogDebug("CREATING NEW PROMISE\n");
    mReady = new rtPromise();
    triggerUpdate();
  }
}

void pxText::dispose(bool pumpJavascript)
{
  removeResourceListener();
  mFont = NULL;
  pxObject::dispose(pumpJavascript);
}

uint64_t pxText::textureMemoryUsage(std::vector<rtObject*> &objectsCounted)
{
  uint64_t textureMemory = 0;
  if (std::find(objectsCounted.begin(), objectsCounted.end(), this) == objectsCounted.end() )
  {
    if (mCached.getPtr() != NULL)
    {
      textureMemory += (mCached->width() * mCached->height() * 4);
    }
    objectsCounted.push_back(this);
  }

  return textureMemory;
}

rtError pxText::setColor(float *var, rtValue c)
{
  // Set via STRING...
  if( c.getType() == 's')
  {
    rtString str = c.toString();

    uint8_t r,g,b, a;
    if( web2rgb( str, r, g, b, a) == RT_OK)
    {
      var[PX_RED  ] = (float) r / 255.0f;  // R
      var[PX_GREEN] = (float) g / 255.0f;  // G
      var[PX_BLUE ] = (float) b / 255.0f;  // B
      var[PX_ALPHA] = (float) a / 255.0f;  // A

      return RT_OK;
    }

    return RT_FAIL;
  }

  // JS is all doubles !
  uint32_t clr = (uint32_t) c.toDouble();

  return colorUInt32_to_Float4(var, clr);
}


rtError pxText::setTextColor(rtValue c)
{
  return setColor(mTextColor, c);
}

rtError pxText::textColor(rtValue &c) const
{
  uint32_t cc = 0;
  rtError err = colorFloat4_to_UInt32(&mTextColor[0], cc);

  c = cc;

  return err;
}

// Helpers
rtError pxText::colorFloat4_to_UInt32( const float *clr, uint32_t& c) const
{
  if(clr == NULL)
  {
    rtLogError("Bad color param. (NULL)");
    return RT_FAIL;
  }
#ifdef PX_LITTLEENDIAN_PIXELS

  c = ((uint8_t) (clr[0] * 255.0f) << 24) |  // R
      ((uint8_t) (clr[1] * 255.0f) << 16) |  // G
      ((uint8_t) (clr[2] * 255.0f) <<  8) |  // B
      ((uint8_t) (clr[3] * 255.0f) <<  0);   // A
#else

  c = ((uint8_t) (clr[3] * 255.0f) << 24) |  // A
      ((uint8_t) (clr[2] * 255.0f) << 16) |  // B
      ((uint8_t) (clr[1] * 255.0f) <<  8) |  // G
      ((uint8_t) (clr[0] * 255.0f) <<  0);   // R
#endif

  return RT_OK;
}

rtError pxText::colorUInt32_to_Float4(float *clr, uint32_t c) const
{
  clr[PX_RED  ] = (float)((c>>24) & 0xff) / 255.0f;
  clr[PX_GREEN] = (float)((c>>16) & 0xff) / 255.0f;
  clr[PX_BLUE ] = (float)((c>> 8) & 0xff) / 255.0f;
  clr[PX_ALPHA] = (float)((c>> 0) & 0xff) / 255.0f;

  return RT_OK;
}


rtDefineObject(pxText, pxObject);

rtDefineProperty(pxText, text);
rtDefineProperty(pxText, textColor);
rtDefineProperty(pxText, pixelSize);
rtDefineProperty(pxText, fontUrl);
rtDefineProperty(pxText, font);
rtDefineMethod(pxText, texture);
