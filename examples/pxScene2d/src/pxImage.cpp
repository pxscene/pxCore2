// pxCore CopyRight 2007-2015 John Robinson
// pxImage.cpp

#include "rtString.h"
#include "rtRef.h"
#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxScene2d.h"
#include "pxOffscreen.h"

#include "pxImage.h"
#include "pxContext.h"
#include "rtFileDownloader.h"

using namespace std;

extern pxContext context;

bool isPowerOfTwo(int64_t number)
{
  return ((number & (number - 1)) == 0);
}

pxImage::~pxImage()
{
  rtLogDebug("~pxImage()");
  if (mListenerAdded)
  {
    if (getImageResource())
    {
      getImageResource()->removeListener(this);
    }
    mResource = NULL;
    mListenerAdded = false;
  }
}

void pxImage::onInit()
{
  //rtLogDebug("pxImage::onInit() mUrl=%s\n",mUrl.cString());
  mInitialized = true;
  //rtLogDebug("pxImage::onInit for mUrl=\n");
  //rtLogDebug("%s\n",getImageResource()->getUrl().cString());
  setUrl(getImageResource()->getUrl());
}

/**
 * setResource
 * */
rtError pxImage::setResource(rtObjectRef o) 
{ 
  //rtLogDebug("!!!!!!!!!!!!!!!!!!!!!pxImage setResource\n");
  if(!o)
  { 
    setUrl("");
    return RT_OK;
  }
  
  // Verify the object passed in is an rtImageResource
  rtString desc;
  o.sendReturns("description",desc);
  if(!desc.compare("rtImageResource"))
  {
    rtString url;
    url = o.get<rtString>("url");
    // Only create new promise if url is different 
    if( getImageResource()->getUrl().compare(o.get<rtString>("url")) )
    {
      mResource = o; 
      imageLoaded = false;
      pxObject::createNewPromise();
      mListenerAdded = true;
      getImageResource()->addListener(this);
      checkStretchX();
      checkStretchY();
    }
    return RT_OK; 
  } 
  else 
  {
    rtLogError("Object passed as resource is not an imageResource!\n");
    return RT_ERROR; 
  }

}

rtError pxImage::url(rtString& s) const { s = getImageResource()->getUrl(); return RT_OK; }
rtError pxImage::setUrl(const char* s) 
{ 
  //rtLogInfo("pxImage::setUrl init=%d imageLoaded=%d \n", mInitialized, imageLoaded);
  //rtLogDebug("pxImage::setUrl for s=%s mUrl=%s\n", s, mUrl.cString());
  
  // we don't want to createNewPromise on the first time through when the 
  // url is initially being set because it's already created on construction
  // If mUrl is already set and loaded and s is different, create a new promise
  rtImageResource* resourceObj = getImageResource();
  if( resourceObj->getUrl().length() > 0 && resourceObj->getUrl().compare(s) && imageLoaded)
  {
    if(imageLoaded) 
    {
      imageLoaded = false;
      //rtLogDebug("pxImage calling pxObject::createPromise for %s\n",resourceObj->getUrl().cString());
      pxObject::createNewPromise();
    }
    //else 
    //{
      //// Stop listening for the old resource that this image was using
      //resourceObj->removeListener(this);
      //mReady.send("reject",this); // reject the original promise for old image
    //}
  }


  mResource = pxImageManager::getImage(s);

  if(getImageResource()->getUrl().length() > 0 && mInitialized && !imageLoaded) {
    mListenerAdded = true;
    getImageResource()->addListener(this);
  }
  
  return RT_OK;
}

void pxImage::sendPromise() 
{ 
  if(mInitialized && imageLoaded && !((rtPromise*)mReady.getPtr())->status()) 
  {
      //rtLogDebug("pxImage SENDPROMISE for %s\n", mUrl.cString());
      mReady.send("resolve",this); 
  }
}

float pxImage::getOnscreenWidth() 
{ 
  if(mw == -1 ) 
  {
    return mResource.get<float>("w");
  }
  else 
    return mw; 

}
float pxImage::getOnscreenHeight() 
{ 
  if(mh == -1) 
  {
    return mResource.get<float>("h");
  }
  else  
    return mh;  
 }
      
void pxImage::draw() {
  //rtLogDebug("pxImage::draw() mw=%f mh=%f\n", mw, mh);
  static pxTextureRef nullMaskRef;
  context.drawImage(0, 0, 
                    getOnscreenWidth(),
                    getOnscreenHeight(), 
                    getImageResource()->getTexture(), nullMaskRef, 
                    false, NULL, mStretchX, mStretchY);
  // Raise the priority if we're still waiting on the image download    
#if 0
  if (!imageLoaded && getImageResource()->isDownloadInProgress())
    getImageResource()->raiseDownloadPriority();
#endif
}
void pxImage::resourceReady(rtString readyResolution)
{
  checkStretchX();
  checkStretchY();
  //rtLogDebug("pxImage::resourceReady(%s) mInitialized=%d for \"%s\"\n",readyResolution.cString(),mInitialized,getImageResource()->getUrl().cString());
  if( !readyResolution.compare("resolve"))
  {
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

void pxImage::dispose()
{
  if (mListenerAdded)
  {
    if (getImageResource())
    {
      getImageResource()->removeListener(this);
    }
    mResource = NULL;
    mListenerAdded = false;
  }
  pxObject::dispose();
}

void pxImage::checkStretchX()
{
  rtImageResource* imageResource = getImageResource();
  if (mStretchX == pxConstantsStretch::REPEAT && imageResource != NULL)
  {
    pxTextureRef texture = imageResource->getTexture();
    if (texture.getPtr() != NULL && (!isPowerOfTwo(texture->width()) || !isPowerOfTwo(texture->height())))
    {
      rtLogWarn("stretchX set to REPEAT but image is not a power of 2 for image: %s", imageResource->getUrl().cString());
    }
  }
}

void pxImage::checkStretchY()
{
  rtImageResource* imageResource = getImageResource();
  if (mStretchY == pxConstantsStretch::REPEAT && imageResource != NULL)
  {
    pxTextureRef texture = imageResource->getTexture();
    if (texture.getPtr() != NULL && (!isPowerOfTwo(texture->width()) || !isPowerOfTwo(texture->height())))
    {
      rtLogWarn("stretchY set to REPEAT but image is not a power of 2 for image: %s", imageResource->getUrl().cString());
    }
  }
}

rtError pxImage::setStretchX(int32_t v)
{
  mStretchX = (pxConstantsStretch::constants)v;
  checkStretchX();
  return RT_OK;
}

rtError pxImage::setStretchY(int32_t v)
{
  mStretchY = (pxConstantsStretch::constants)v;
  checkStretchY();
  return RT_OK;
}

rtDefineObject(pxImage,pxObject);
rtDefineProperty(pxImage,url);
rtDefineProperty(pxImage, resource);
rtDefineProperty(pxImage,stretchX);
rtDefineProperty(pxImage,stretchY);


