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
  rtLogDebug("pxImage::onInit() mURL=%s\n",mURL.cString());
  mInitialized = true;
  setURL(mURL);
}

rtError pxImage::url(rtString& s) const { s = mURL; return RT_OK; }
rtError pxImage::setURL(const char* s) 
{ 
  rtLogDebug("pxImage::setURL init=%d imageLoaded=%d s=%s mURL=%s\n", mInitialized, imageLoaded, s, mURL.cString());
  
  // we don't want to createNewPromise on the first time through when the 
  // url is initially being set because it's already created on construction
  // If mURL is already set and loaded and s is different, create a new promise
  if( mURL.length() > 0 && mURL.compare(s) && imageLoaded)
  {
    imageLoaded = false;
    rtLogDebug("pxImage calling pxObject::createPromise for %s\n",mURL.cString());
    pxObject::createNewPromise();
  }
  mURL = s;
  if (!s || !u8_strlen((char*)s)) 
    return RT_OK;
  if (mInitialized)
    loadImage(mURL);
  else
    rtLogDebug("Deferring image load until pxImage is initialized.");
  return RT_OK;
}

void pxImage::loadImage(rtString url)
{
  //printf("pxImage::loadImage %s\n",url.cString());
  mTextureCacheObject.setURL(url);
}

void pxImage::sendPromise() 
{ 
  if(mInitialized && (imageLoaded || mStatusCode!=RT_OK) && !((rtPromise*)mReady.getPtr())->status()) 
  {
      rtLogDebug("pxImage SENDPROMISE for %s\n", mURL.cString());
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
    {
      // Send the promise here because the image will not get an 
      // update call until it has a parent
      sendPromise();
      rtLogDebug("In pxImage::onTextureReady, pxImage with url=%s has no parent!\n", mURL.cString());
    }
     
    ////// send after width and height have been set
    // TO DO: Remove use of onReady in samples
    //rtObjectRef e = new rtMapObject;
    //e.set("name", "onReady");
    //e.set("target", this);
    //mEmit.send("onReady", e);
   
    
    return true;
  }
  rtLogWarn("pxImage SENDPROMISE for ERROR %s\n", mURL.cString());
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


