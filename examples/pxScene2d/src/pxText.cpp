// pxCore CopyRight 2007-2015 John Robinson
// pxText.cpp

#include "pxText.h"
#include "pxFileDownloader.h"
#include "pxTimer.h"
#include "pxFont.h"
#include "pxContext.h"

extern pxContext context;


pxText::pxText(pxScene2d* scene):pxObject(scene)
{
  float c[4] = {1, 1, 1, 1};
  memcpy(mTextColor, c, sizeof(mTextColor));
  // Default to use default font
  mFont = pxFontManager::getFont(scene,defaultFont);
  mPixelSize = defaultPixelSize;
  mDirty = true;
}


void pxText::onInit()
{

  mInitialized = true;

  if( getFontResource()->isFontLoaded()) {
    fontLoaded("resolve");
  }
}
rtError pxText::text(rtString& s) const { s = mText; return RT_OK; }

void pxText::sendPromise() 
{ 
  if(mInitialized && mFontLoaded && !((rtPromise*)mReady.getPtr())->status()) 
  {
    //printf("pxText SENDPROMISE\n");
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
  createNewPromise();
  getFontResource()->measureTextInternal(s, mPixelSize, 1.0, 1.0, mw, mh);
  return RT_OK; 
}

rtError pxText::setPixelSize(uint32_t v) 
{   
  //rtLogInfo("pxText::setPixelSize\n");
  mPixelSize = v; 
  createNewPromise();
  getFontResource()->measureTextInternal(mText, mPixelSize, 1.0, 1.0, mw, mh);
  return RT_OK; 
}

void pxText::fontLoaded(const char * /*value*/)
{
  printf("pxText::fontLoaded for text value=%s\n",mText.cString());
//  rtLogDebug("pxText::fontLoaded for fontName=%s and mInitialized=%d\n",mFontUrl.compare("")?mFontUrl.cString():defaultFont, mInitialized); 
  mFontLoaded=true;
  // pxText gets its height and width from the text itself, 
  // so measure it
  getFontResource()->measureTextInternal(mText, mPixelSize, 1.0, 1.0, mw, mh);
  mDirty=true;  
//  printf("After fontLoaded and measureText, mw=%f and mh=%f\n",mw,mh);
  
  if( mInitialized) {
    // This repaint logic is necessary for clearing FBO if
    // clipping is on
    repaint();
    pxObject* parent = mParent;
    while (parent)
    {
     parent->repaint();
     parent = parent->parent();
    }    
  }  
  
}
        
void pxText::update(double t)
{
  pxObject::update(t);
  
#if 1
  if (mDirty)
  {
#if 0
    // TODO magic number
    if (mText.length() >= 5)
    {
      setPainting(true);
      setPainting(false);
    }
    else
      setPainting(true);
#else
    // TODO make this configurable
    if (mText.length() >= 10)
    {
      mCached = NULL;
      pxContextFramebufferRef cached = context.createFramebuffer(getFBOWidth(),getFBOHeight());//mw,mh);
      if (cached.getPtr())
      {
        pxContextFramebufferRef previousSurface = context.getCurrentFramebuffer();
        context.setFramebuffer(cached);
        pxMatrix4f m;
        context.setMatrix(m);
        context.setAlpha(1.0);
        context.clear(mw,mh);
        draw();
        context.setFramebuffer(previousSurface);
        mCached = cached;
      }
    }
    else mCached = NULL;
    
#endif
    
    mDirty = false;
    }
#else
  mDirty = false;
#endif
  
}

void pxText::draw() {
  static pxTextureRef nullMaskRef;
  if (mCached.getPtr() && mCached->getTexture().getPtr())
  {
    context.drawImage(0, 0, mw, mh, mCached->getTexture(), nullMaskRef, PX_NONE, PX_NONE);
  }
  else
  {
    getFontResource()->renderText(mText, mPixelSize, 0, 0, 1.0, 1.0, mTextColor, mw);
  }
}

rtError pxText::setFontUrl(const char* s)
{
  //printf("pxText::setFaceUrl for %s\n",s);
  if (!s || !s[0]) {
    s = defaultFont;
  }
  mFontLoaded = false;
  createNewPromise();
//  mFontUrl = s;

  mFont = pxFontManager::getFont(mScene, s);
  getFontResource()->addListener(this);
  
  return RT_OK;
}

rtDefineObject(pxText, pxObject);
rtDefineProperty(pxText, text);
rtDefineProperty(pxText, textColor);
rtDefineProperty(pxText, pixelSize);
rtDefineProperty(pxText, fontUrl);
rtDefineProperty(pxText, font);
