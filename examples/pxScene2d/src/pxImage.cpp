// pxCore CopyRight 2007-2015 John Robinson
// pxImage.cpp

#include "rtString.h"
#include "rtRefT.h"
#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxScene2d.h"
#include "pxOffscreen.h"

#include "pxImage.h"

#include "pxContext.h"

#include "pxFileDownloader.h"

extern "C"
{
#include "utf8.h"
}

#include <map>
using namespace std;

extern pxContext context;

void pxImage::onInit()
{
  rtLogDebug("pxImage::onInit() mUrl=%s\n",mUrl.cString());
  mInitialized = true;
  setUrl(mUrl);
}

rtError pxImage::url(rtString& s) const { s = mUrl; return RT_OK; }
rtError pxImage::setUrl(const char* s) 
{ 
  rtLogDebug("pxImage::setUrl init=%d imageLoaded=%d s=%s mUrl=%s\n", mInitialized, imageLoaded, s, mUrl.cString());
  
  // we don't want to createNewPromise on the first time through when the 
  // url is initially being set because it's already created on construction
  // If mUrl is already set and loaded and s is different, create a new promise
  if( mUrl.length() > 0 && mUrl.compare(s) && imageLoaded)
  {
    imageLoaded = false;
    rtLogDebug("pxImage calling pxObject::createPromise for %s\n",mUrl.cString());
    pxObject::createNewPromise();
  }
  mUrl = s;
  if (!s || !u8_strlen((char*)s)) 
    return RT_OK;
  if (mInitialized)
    loadImage(mUrl);
  else
    rtLogDebug("Deferring image load until pxImage is initialized.");
  return RT_OK;
}

void pxImage::loadImage(rtString url)
{
  //printf("pxImage::loadImage %s\n",url.cString());
  mTextureCacheObject.setUrl(url);
}

void pxImage::sendPromise() 
{ 
  if(mInitialized && (imageLoaded || mStatusCode!=RT_OK) && !((rtPromise*)mReady.getPtr())->status()) 
  {
      rtLogDebug("pxImage SENDPROMISE for %s\n", mUrl.cString());
      mReady.send("resolve",this); }
  }
    
void pxImage::draw() {
  static pxTextureRef nullMaskRef;
  context.drawImage(0, 0, mw, mh, mTexture, nullMaskRef, 
                    mXStretch, mYStretch);
  if (mTextureCacheObject.isDownloadInProgress())
  {
    mTextureCacheObject.raiseDownloadPriority();
  }
}

bool pxImage::onTextureReady(pxTextureCacheObject* textureCacheObject, rtError status)
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
  if (textureCacheObject != NULL && status == RT_OK)
  {
    imageLoaded = true; 
    mTexture = textureCacheObject->getTexture();
    if (mAutoSize && mTexture.getPtr() != NULL)
    {
      mw = mTexture->width();
      mh = mTexture->height();
    } 

    pxObject* parent = mParent;
    if( !parent)
      rtLogWarn("In pxImage::onTextureReady, pxImage with url=%s has no parent!\n", mUrl.cString());
     
    ////// send after width and height have been set
    // TO DO: Remove use of onReady in samples
    //rtObjectRef e = new rtMapObject;
    //e.set("name", "onReady");
    //e.set("target", this);
    //mEmit.send("onReady", e);
    // !CLF:  WHY doesn't the instant info image draw without this
    // sendPromise()? Why is it not being called from the update loop?? 
    //sendPromise();
   
    
    return true;
  }
  rtLogWarn("pxImage SENDPROMISE for ERROR %s\n", mUrl.cString());
  mReady.send("reject",this);
  return false;
}

rtDefineObject(pxImage,pxObject);
rtDefineProperty(pxImage,url);
rtDefineProperty(pxImage,xStretch);
rtDefineProperty(pxImage,yStretch);
rtDefineProperty(pxImage,autoSize);
rtDefineProperty(pxImage,statusCode);
rtDefineProperty(pxImage,httpStatusCode);


