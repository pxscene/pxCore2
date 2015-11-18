// pxCore CopyRight 2007-2015 John Robinson
// pxText.cpp

#include "pxText.h"
#include "pxFileDownloader.h"
#include "pxTimer.h"
#include "pxFont.h"

#include <math.h>
#include <map>





#include "pxContext.h"

extern pxContext context;


pxText::pxText(pxScene2d* scene):pxObject(scene)//, mFontDownloadRequest(NULL)
{
  float c[4] = {1, 1, 1, 1};
  memcpy(mTextColor, c, sizeof(mTextColor));
  mFont = pxFontManager::getFontObj(scene,defaultFace);
//  mFace = gFace;
  mPixelSize = defaultPixelSize;
  mDirty = true;
}

pxText::~pxText()
{
  //if (mFontDownloadRequest != NULL)
  //{
    //// if there is a previous request pending then set the callback to NULL
    //// the previous request will not be processed and the memory will be freed when the download is complete
    //mFontDownloadRequest->setCallbackFunctionThreadSafe(NULL);
  //}
}

void pxText::onInit()
{
//  printf("pxText2::onInit. mFontLoaded=%d\n",mFontLoaded);
  mInitialized = true;

  if( mFont->isFontLoaded()) {

    fontLoaded("resolve");
  }
}
rtError pxText::text(rtString& s) const { s = mText; return RT_OK; }

rtError pxText::setText(const char* s) { 
  printf("pxText::setText\n"); 
  mText = s; 
  mFont->getFace()->measureText(s, mPixelSize, 1.0, 1.0, mw, mh);
  return RT_OK; 
}

rtError pxText::setPixelSize(uint32_t v) 
{   
  printf("pxText::setPixelSize\n"); 
  mPixelSize = v; 
  mFont->getFace()->measureText(mText, mPixelSize, 1.0, 1.0, mw, mh);
  return RT_OK; 
}

void pxText::fontLoaded(const char * value)
{
  printf("pxText::fontLoaded for fontFace=%s and mInitialized=%d\n",mFaceURL.cString(), mInitialized); 
  mFontLoaded=true;
  // pxText gets its height and width from the text itself, 
  // so measure it
  mFont->getFace()->measureText(mText, mPixelSize, 1.0, 1.0, mw, mh);
  mDirty=true;  
  printf("After fontLoaded and measureText, mw=%f and mh=%f\n",mw,mh);
  
  if( mInitialized) {
    mReady.send(value, this);
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
    mFont->getFace()->renderText(mText, mPixelSize, 0, 0, 1.0, 1.0, mTextColor, mw);
  }
}

rtError pxText::setFaceURL(const char* s)
{
  printf("pxText::setFaceURL for %s\n",s);
  if (!s || !s[0]) {
    s = defaultFace;
  }
  mFontLoaded = false;
  mFaceURL = s;

  mFont = pxFontManager::getFontObj(mScene, s);
  mFont->addListener(this);
  
  return RT_OK;
}

rtDefineObject(pxText, pxObject);
rtDefineProperty(pxText, text);
rtDefineProperty(pxText, textColor);
rtDefineProperty(pxText, pixelSize);
rtDefineProperty(pxText, faceURL);
