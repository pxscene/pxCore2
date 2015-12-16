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
  mFont = pxFontManager::getFont(scene,defaultFace);
  mPixelSize = defaultPixelSize;
  mDirty = true;
}


void pxText::onInit()
{
  //printf("pxText2::onInit. mFontLoaded=%d\n",mFontLoaded);
  mInitialized = true;

  if( mFont->isFontLoaded()) {
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
  mText = s; 
  createNewPromise();
  mFont->measureText(s, mPixelSize, 1.0, 1.0, mw, mh);
  return RT_OK; 
}

rtError pxText::setPixelSize(uint32_t v) 
{   
  mPixelSize = v; 
  createNewPromise();
  mFont->measureText(mText, mPixelSize, 1.0, 1.0, mw, mh);
  return RT_OK; 
}

void pxText::fontLoaded(const char * /*value*/)
{
  //rtLogInfo("pxText::fontLoaded for fontFace=%s and mInitialized=%d\n",mFaceURL.compare("")?mFaceURL.cString():defaultFace, mInitialized); 
  mFontLoaded=true;
  // pxText gets its height and width from the text itself, 
  // so measure it
  mFont->measureText(mText, mPixelSize, 1.0, 1.0, mw, mh);
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
    mFont->renderText(mText, mPixelSize, 0, 0, 1.0, 1.0, mTextColor, mw);
  }
}

rtError pxText::setFaceURL(const char* s)
{
  //printf("pxText::setFaceURL for %s\n",s);
  if (!s || !s[0]) {
    s = defaultFace;
  }
  mFontLoaded = false;
  createNewPromise();
  mFaceURL = s;

  mFont = pxFontManager::getFont(mScene, s);
  mFont->addListener(this);
  
  return RT_OK;
}

rtDefineObject(pxText, pxObject);
rtDefineProperty(pxText, text);
rtDefineProperty(pxText, textColor);
rtDefineProperty(pxText, pixelSize);
rtDefineProperty(pxText, faceURL);
