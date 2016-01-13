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

void pxImage9::onInit()
{
  mInitialized = true;
  setUrl(mUrl);
}

rtError pxImage9::url(rtString& s) const 
{ 
  s = mUrl; 
  return RT_OK; 
}

rtError pxImage9::setUrl(const char* s) { 
  
  if(mUrl.length() > 0 && mUrl.compare(s) && imageLoaded)
  {
    imageLoaded = false;
    pxObject::createNewPromise();
  }      
  mUrl = s;
  getImageResource()->setUrl(s);

  if (!s || !u8_strlen((char*)s)) 
    return RT_OK;  
    
  if(mInitialized)
    loadImage(mUrl);
    
  return RT_OK;
}

void pxImage9::sendPromise() 
{ 
  //printf("image9 init=%d imageLoaded=%d\n",mInitialized,imageLoaded);
  if(mInitialized && imageLoaded && !((rtPromise*)mReady.getPtr())->status()) 
  { 
    rtLogDebug("pxImage9 SENDPROMISE for %s\n", mUrl.cString()); 
    mReady.send("resolve",this);
  } 
}


void pxImage9::draw() {
  context.drawImage9(mw, mh, mInsetLeft, mInsetTop, mInsetRight, mInsetBottom, mTextureCacheObject.getTexture());
}

void pxImage9::loadImage(rtString url)
{
  mTextureCacheObject.setUrl(url);
}

bool pxImage9::onTextureReady(pxTextureCacheObject* textureCacheObject, rtError status)
{
  if (pxObject::onTextureReady(textureCacheObject, status))
  {
    imageLoaded = true;
    return true;
  }

  if (textureCacheObject != NULL)
  {
    getImageResource()->setLoadStatus("statusCode", textureCacheObject->getStatusCode());
    getImageResource()->setLoadStatus("httpStatusCode", textureCacheObject->getHttpStatusCode());    
  }
  pxObject* parent = mParent;
  if( !parent)
  {
    // Send the promise here because the image will not get an 
    // update call until it has a parent
    sendPromise();
    rtLogDebug("In pxImage9::onTextureReady, pxImage with url=%s has no parent!\n", mUrl.cString());
  }
  if (textureCacheObject != NULL && status == RT_OK && textureCacheObject->getTexture().getPtr() != NULL)
  {
    mw = textureCacheObject->getTexture()->width();
    mh = textureCacheObject->getTexture()->height();
    imageLoaded = true;
    return true;
  }
  getImageResource()->sendPromise("reject");
  mReady.send("reject",this);
  return false;
}

rtDefineObject(pxImage9, pxObject);
rtDefineProperty(pxImage9, url);
rtDefineProperty(pxImage9, insetLeft);
rtDefineProperty(pxImage9, insetTop);
rtDefineProperty(pxImage9, insetRight);
rtDefineProperty(pxImage9, insetBottom);
