// pxCore CopyRight 2007-2015 John Robinson
// pxImage9.cpp

#include "rtString.h"
#include "rtRefT.h"
#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxScene2d.h"
#include "pxImage9.h"
#include "pxContext.h"
#include "pxFileDownloader.h"

extern "C"
{
#include "utf8.h"
}

extern pxContext context;

pxImage9::~pxImage9()
{
  rtLogDebug("~pxImage9()");
  if (mListenerAdded)
  {
    if (getImageResource())
    {
      getImageResource()->removeListener(this);
    }
    mListenerAdded = false;
  }
}

void pxImage9::onInit()
{
  mInitialized = true;
  setUrl(getImageResource()->getUrl());
}

rtError pxImage9::url(rtString& s) const 
{ 
  s = getImageResource()->getUrl(); 
  return RT_OK; 
}

rtError pxImage9::setUrl(const char* s) 
{ 
  rtImageResource* resourceObj = getImageResource();  
  if(resourceObj->getUrl().length() > 0 && resourceObj->getUrl().compare(s) && imageLoaded)
  {
    imageLoaded = false;
    pxObject::createNewPromise();
  } 
  
  if (mListenerAdded)
  {
    if (getImageResource())
    {
      getImageResource()->removeListener(this);
    }
    mListenerAdded = false;
  }

  mResource = pxImageManager::getImage(s); 
  if(getImageResource()->getUrl().length() > 0)
  {
    mListenerAdded = true;
    getImageResource()->addListener(this);
  }
    
  return RT_OK;
}

void pxImage9::sendPromise() 
{ 
  //printf("image9 init=%d imageLoaded=%d\n",mInitialized,imageLoaded);
  if(mInitialized && imageLoaded && !((rtPromise*)mReady.getPtr())->status()) 
  { 
    rtLogDebug("pxImage9 SENDPROMISE for %s\n", getImageResource()->getUrl().cString()); 
    mReady.send("resolve",this);
  } 
}


void pxImage9::draw() {
  context.drawImage9(mw, mh, mInsetLeft, mInsetTop, mInsetRight, mInsetBottom, getImageResource()->getTexture());
}

void pxImage9::resourceReady(rtString readyResolution)
{
  //printf("pxImage9::resourceReady()\n");
  if( !readyResolution.compare("resolve"))
  {
    imageLoaded = true; 
    // nineslice gets its w and h from the image only
    mw = getImageResource()->w();
    mh = getImageResource()->h();
    imageLoaded = true;
    pxObject::onTextureReady();
    // Now that image is loaded, must force redraw;
    // dimensions could have changed.
    mScene->mDirty = true;
    pxObject* parent = mParent;
    if( !parent)
    {
      // Send the promise here because the image will not get an 
      // update call until it has a parent
      sendPromise();
      //rtLogInfo("In pxImage::resourceReady, pxImage with url=%s has no parent!\n", getImageResource()->getUrl().cString());
    }
  }
  else 
  {
      pxObject::onTextureReady();
      mReady.send("reject",this);
  }
}

rtDefineObject(pxImage9, pxObject);
rtDefineProperty(pxImage9, url);
rtDefineProperty(pxImage9, insetLeft);
rtDefineProperty(pxImage9, insetTop);
rtDefineProperty(pxImage9, insetRight);
rtDefineProperty(pxImage9, insetBottom);
rtDefineProperty(pxImage9, resource);
