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

extern pxContext context;

pxText::pxText(pxScene2d* scene):pxObject(scene), mFontLoaded(false), mFontFailed(false), mDirty(true), mFontDownloadRequest(NULL), mListenerAdded(false)

{
  float c[4] = {1, 1, 1, 1};
  memcpy(mTextColor, c, sizeof(mTextColor));
  // Default to use default font
  mFont = pxFontManager::getFont(defaultFont, NULL, NULL, scene->getArchive());
  mPixelSize = defaultPixelSize;
}

pxText::~pxText()
{
  removeResourceListener();
}

void pxText::onInit()
{
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
    createNewPromise();
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
    createNewPromise();
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
    }
#ifdef PXSCENE_FONT_ATLAS
    if (mDirty)
    {
      getFontResource()->renderTextToQuads(mText,mPixelSize,msx,msy,mQuads);
      mDirty = false;
    }
    mQuads.draw(0,0,mTextColor);
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
        context.drawImage(0, 0, (mw>MAX_TEXTURE_WIDTH?MAX_TEXTURE_WIDTH:mw), (mh>MAX_TEXTURE_HEIGHT?MAX_TEXTURE_HEIGHT:mh), cached->getTexture(), nullMaskRef);
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
  mFont = pxFontManager::getFont(s, NULL, NULL, mScene->getArchive());
  mListenerAdded = true;
  if (getFontResource() != NULL)
  {
    getFontResource()->addListener(this);
  }
  
  return RT_OK;
}

rtError pxText::setFont(rtObjectRef o) 
{ 
  mFontLoaded = false;
  mFontFailed = false;
  createNewPromise();

  // !CLF: TODO: Need validation/verification of o
  removeResourceListener();
  mFont = o; 
  mListenerAdded = true;
  if (getFontResource() != NULL) {
    getFontResource()->addListener(this);
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
  }
}

void pxText::dispose(bool pumpJavascript)
{
  removeResourceListener();
  mFont = NULL;
  pxObject::dispose(pumpJavascript);
}

uint64_t pxText::textureMemoryUsage()
{
  uint64_t textureMemory = 0;
  if (mCached.getPtr() != NULL)
  {
    textureMemory += (mCached->width() * mCached->height() * 4);
  }

  return textureMemory;
}


rtDefineObject(pxText, pxObject);
rtDefineProperty(pxText, text);
rtDefineProperty(pxText, textColor);
rtDefineProperty(pxText, pixelSize);
rtDefineProperty(pxText, fontUrl);
rtDefineProperty(pxText, font);
