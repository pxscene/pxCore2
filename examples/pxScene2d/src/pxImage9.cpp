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
  setURL(mURL);
}

rtError pxImage9::url(rtString& s) const 
{ 
  s = mURL; 
  return RT_OK; 
}

rtError pxImage9::setURL(const char* s) { 
  
  if(mURL.length() > 0 && mURL.compare(s) && imageLoaded)
  {
    imageLoaded = false;
    pxObject::createNewPromise();
  }      
  mURL = s;

  if (!s || !u8_strlen((char*)s)) 
    return RT_OK;  
    
  if(mInitialized)
    loadImage(mURL);
    
  return RT_OK;
}

void pxImage9::sendPromise() 
{ 
  //printf("image9 init=%d imageLoaded=%d\n",mInitialized,imageLoaded);
  if(mInitialized && imageLoaded && !((rtPromise*)mReady.getPtr())->status()) 
  { 
    rtLogDebug("pxImage9 SENDPROMISE for %s\n", mURL.cString()); 
    mReady.send("resolve",this);
  } 
}


void pxImage9::draw() {
  context.drawImage9(mw, mh, ml, mt, mr, mb, mTextureCacheObject.getTexture());
}

void pxImage9::loadImage(rtString url)
{
  mTextureCacheObject.setURL(url);
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
    mStatusCode = textureCacheObject->getStatusCode();
    mHttpStatusCode = textureCacheObject->getHttpStatusCode();
  }
  
  pxObject* parent = mParent;
  if( !parent)
  {
    // Send the promise here because the image will not get an 
    // update call until it has a parent
    sendPromise();
    rtLogWarn("In pxImage9::onTextureReady, pxImage with url=%s has no parent!\n", mURL.cString());
  }
  
  if (textureCacheObject != NULL && status == RT_OK && textureCacheObject->getTexture().getPtr() != NULL)
  {
    mw = textureCacheObject->getTexture()->width();
    mh = textureCacheObject->getTexture()->height();
    imageLoaded = true;
    return true;
  }
  mReady.send("reject",this);
  return false;
}

rtDefineObject(pxImage9, pxObject);
rtDefineProperty(pxImage9, url);
rtDefineProperty(pxImage9, lInset);
rtDefineProperty(pxImage9, tInset);
rtDefineProperty(pxImage9, rInset);
rtDefineProperty(pxImage9, bInset);
rtDefineProperty(pxImage9,statusCode);
rtDefineProperty(pxImage9,httpStatusCode);
