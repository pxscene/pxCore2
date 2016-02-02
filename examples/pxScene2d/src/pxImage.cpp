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

//#include <map>
using namespace std;

extern pxContext context;

void pxImage::onInit()
{
  //rtLogDebug("pxImage::onInit() mUrl=%s\n",mUrl.cString());
  mInitialized = true;
  //printf("pxImage::onInit for mUrl=\n");
  //printf("%s\n",getImageResource()->getUrl().cString());
  setUrl(getImageResource()->getUrl());
}

/**
 * !CLF: Should setResource check if url matches existing resource url, 
 * and just send promise if that is the case?
 * */
rtError pxImage::setResource(rtObjectRef o) 
{ 
  //printf("!!!!!!!!!!!!!!!!!!!!!pxImage setResource\n");
  //rtString desc;
  //printf("!!!!!!!!!!!!!!!!!!!!!pxImage setResource about to do static cast\n");
  //// Without try/catch handling, how do we find out whether this rtObjectRef is eligible or not???
  //rtError err = static_cast<rtObjectRef &>(o).sendReturns<rtString>("description", desc);
  //printf("desc is %s\n",desc.cString());
  //if (err == RT_OK && strcmp(desc.cString(), "rtImageResource") == 0)
  //{
    //printf("Object is an rtImageResource\n");
    mResource = o; 
    imageLoaded = false;
    //printf("image promise state is %d\n",((rtPromise*)mReady.getPtr())->status());
    pxObject::createNewPromise();
    getImageResource()->addListener(this); 
    return RT_OK; 
  //}
  //else 
  //{
    //rtLogError("Object passed as resource is not an imageResource!\n");
    //return RT_OK;
  //}

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
                    mStretchX, mStretchY);
  // Raise the priority if we're still waiting on the image download                  
  if (!imageLoaded && getImageResource()->isDownloadInProgress())
    getImageResource()->raiseDownloadPriority();

}
void pxImage::resourceReady(rtString readyResolution)
{
  //printf("pxImage::resourceReady(%s) mInitialized=%d for \"%s\"\n",readyResolution.cString(),mInitialized,getImageResource()->getUrl().cString());
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

rtDefineObject(pxImage,pxObject);
rtDefineProperty(pxImage,url);
rtDefineProperty(pxImage, resource);
rtDefineProperty(pxImage,stretchX);
rtDefineProperty(pxImage,stretchY);


