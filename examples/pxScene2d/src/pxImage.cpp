/*

 pxCore Copyright 2005-2017 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// pxImage.cpp

#include "rtString.h"
#include "rtRef.h"
#include "rtFileDownloader.h"

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxScene2d.h"

// TODO why does pxfont refer to pxImage.h
#include "pxImage.h"
#include "pxContext.h"

using namespace std;

extern pxContext context;

bool isPowerOfTwo(int64_t number)
{
  return ((number & (number - 1)) == 0);
}

pxImage::~pxImage()
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
}

void pxImage::onInit()
{
  //rtLogDebug("pxImage::onInit() mUrl=%s\n",mUrl.cString());
  mInitialized = true;
  //rtLogDebug("pxImage::onInit for mUrl=\n");
  //rtLogDebug("%s\n",getImageResource()->getUrl().cString());
  if (getImageResource() != NULL)
  {
    setUrl(getImageResource()->getUrl());
  }
  else
  {
    setUrl("");
  }
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
    if( getImageResource() != NULL && getImageResource()->getUrl().compare(o.get<rtString>("url")) )
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
    pxObject::onTextureReady();
    mReady.send("reject",this);
    return RT_ERROR; 
  }

}

rtError pxImage::url(rtString& s) const
{
  if (getImageResource() != NULL)
  {
    s = getImageResource()->getUrl();
  }
  else
  {
    s = "";
  }
  return RT_OK;
}
rtError pxImage::setUrl(const char* s)
{ 
  //rtLogInfo("pxImage::setUrl init=%d imageLoaded=%d \n", mInitialized, imageLoaded);
  //rtLogDebug("pxImage::setUrl for s=%s mUrl=%s\n", s, mUrl.cString());
  
  // we don't want to createNewPromise on the first time through when the 
  // url is initially being set because it's already created on construction
  // If mUrl is already set and loaded and s is different, create a new promise
  rtImageResource* resourceObj = getImageResource();
  if( resourceObj != NULL && resourceObj->getUrl().length() > 0 && resourceObj->getUrl().compare(s))
  {
    // This could be an error case where the url was invalid and promise was rejected.
    // If promise was already fulfilled/rejected, create a new one since the url is changing
    if(imageLoaded || ((rtPromise*)mReady.getPtr())->status())
    {
      imageLoaded = false;
      //rtLogDebug("pxImage calling pxObject::createPromise for %s\n",resourceObj->getUrl().cString());
      pxObject::createNewPromise();
    }
    // ToDo Need to cancel the download if url is reassigned before its done
    /*else if(!imageLoaded)
    {
      // Stop listening for the old resource that this image was using
      resourceObj->removeListener(this);
      mReady.send("reject",this); // reject the original promise for old image
    } */
  }



  mResource = pxImageManager::getImage(s);

  if(getImageResource() != NULL && getImageResource()->getUrl().length() > 0 && mInitialized && !imageLoaded) {
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
  if(mw == -1 || mStretchX == pxConstantsStretch::NONE)
  {
    return mResource.get<float>("w");
  }
  else 
    return mw; 

}
float pxImage::getOnscreenHeight() 
{ 
  if(mh == -1 || mStretchY == pxConstantsStretch::NONE)
  {
    return mResource.get<float>("h");
  }
  else  
    return mh;  
}
      
void pxImage::draw() {
  //rtLogDebug("pxImage::draw() mw=%f mh=%f\n", mw, mh);
  static pxTextureRef nullMaskRef;
  if (getImageResource() != NULL)
  {
    context.drawImage(0, 0,
                      getOnscreenWidth(),
                      getOnscreenHeight(),
                      getImageResource()->getTexture(), nullMaskRef,
                      false, NULL, mStretchX, mStretchY, mDownscaleSmooth);
  }
  // Raise the priority if we're still waiting on the image download    
#if 0
  if (!imageLoaded && getImageResource() != NULL && getImageResource()->isDownloadInProgress())
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

rtError pxImage::downscaleSmooth(bool& v) const
{
    v = mDownscaleSmooth;
    return RT_OK;
}

rtError pxImage::setDownscaleSmooth(bool v)
{
    mDownscaleSmooth = v;
    return RT_OK;
}

rtDefineObject(pxImage,pxObject);
rtDefineProperty(pxImage,url);
rtDefineProperty(pxImage, resource);
rtDefineProperty(pxImage,stretchX);
rtDefineProperty(pxImage,stretchY);
rtDefineProperty(pxImage,downscaleSmooth);


