/*

 pxCore Copyright 2005-2018 John Robinson

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
#include <algorithm>

using namespace std;

extern pxContext context;

bool isPowerOfTwo(int64_t number)
{
  return ((number & (number - 1)) == 0);
}

pxImage::~pxImage()
{
  rtLogDebug("~pxImage()");
  removeResourceListener();
  mResource = NULL;
}

void pxImage::onInit()
{
  //rtLogDebug("pxImage::onInit() mUrl=%s\n",mUrl.cString());
  mInitialized = true;
  //rtLogDebug("pxImage::onInit for mUrl=\n");
  //rtLogDebug("%s\n",getImageResource()->getUrl().cString());

  rtImageResource *pRes = getImageResource();

  if (pRes != NULL)
  {
    setUrl(pRes->getUrl());
  }
  else
  {
    setUrl("");
  }
  // send resolve when resource got ready before init
  if ((mParent == NULL) && (mReceivedReadyBeforeInit == true)) {
    mReady.send("resolve",this);
    mReceivedReadyBeforeInit = false;
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
      removeResourceListener();
      mResource = o; 
      imageLoaded = false;
      createNewPromise();
      mListenerAdded = true;
      getImageResource()->addListener(this);
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

void pxImage::createWithOffscreen(pxOffscreen &o)
{
  removeResourceListener();
  mResource = new rtImageResource();
  rtImageResource* imageResource = getImageResource();
  imageResource->createWithOffscreen(o);
  createNewPromise();
  resourceReady("resolve");
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
#ifdef ENABLE_PERMISSIONS_CHECK
  if (mScene != NULL && RT_OK != mScene->permissions()->allows(s, rtPermissions::DEFAULT))
    return RT_ERROR_NOT_ALLOWED;
#endif

  //rtLogInfo("pxImage::setUrl init=%d imageLoaded=%d \n", mInitialized, imageLoaded);
  //rtLogDebug("pxImage::setUrl for s=%s mUrl=%s\n", s, mUrl.cString());
  
  // we don't want to createNewPromise on the first time through when the 
  // url is initially being set because it's already created on construction
  // If mUrl is already set and loaded and s is different, create a new promise
  rtImageResource* pRes = getImageResource();
  if( pRes != NULL && pRes->getUrl().length() > 0 && pRes->getUrl().compare(s))
  {
    // This could be an error case where the url was invalid and promise was rejected.
    // If promise was already fulfilled/rejected, create a new one since the url is changing
    if(imageLoaded || ((rtPromise*)mReady.getPtr())->status())
    {
      imageLoaded = false;
      //rtLogDebug("pxImage calling pxObject::createPromise for %s\n",resourceObj->getUrl().cString());
      createNewPromise();
    }
    // ToDo Need to cancel the download if url is reassigned before its done
    /*else if(!imageLoaded)
    {
      // Stop listening for the old resource that this image was using
      pRes->removeListener(this);
      mReady.send("reject",this); // reject the original promise for old image
    } */
    removeResourceListener();
  }

  if(pRes && !imageLoaded)
  {
    mResource = pxImageManager::getImage(s, NULL, mScene ? mScene->cors() : NULL,
                                                  pRes->initW(),  pRes->initH(),
                                                  pRes->initSX(), pRes->initSY(), mScene ? mScene->getArchive() : NULL, mFlip );
  }

  if(getImageResource() != NULL && getImageResource()->getUrl().length() > 0 && mInitialized && !imageLoaded)
{
    mListenerAdded = true;
    getImageResource()->addListener(this);
  }
  
  return RT_OK;
}

void pxImage::sendPromise()
{
  if(imageLoaded && !((rtPromise*)mReady.getPtr())->status())
  {
      //rtLogDebug("pxImage SENDPROMISE for %s\n", mUrl.cString());
      if (mInitialized)
      {
        mReady.send("resolve",this); 
        mReceivedReadyBeforeInit = false;
      }
      else
      {
        // Received a case where image is loaded before init is done
        mReceivedReadyBeforeInit = true;
      }
  }
}

void pxImage::createNewPromise()
{
  // Only create a new promise if the existing one has been
  // resolved or rejected already.
  if(((rtPromise*)mReady.getPtr())->status())
  {
    rtLogDebug("CREATING NEW PROMISE\n");
    mReady = new rtPromise();
  }
}

bool pxImage::needsUpdate()
{
  if ((mParent != NULL && mAnimations.size() > 0) || (imageLoaded && !((rtPromise*)mReady.getPtr())->status()))
  {
    return true;
  }
  return false;
}

float pxImage::getOnscreenWidth()
{
  if(mw == -1 || mStretchX == pxContextStretch::NONE)
  {
    return mResource.get<float>("w");
  }
  else 
    return mw; 

}
float pxImage::getOnscreenHeight() 
{ 
  if(mh == -1 || mStretchY == pxContextStretch::NONE)
  {
    return mResource.get<float>("h");
  }
  else  
    return mh;  
}
      
void pxImage::draw() {
  //rtLogDebug("pxImage::draw() mw=%f mh=%f\n", mw, mh);
  static pxTextureRef nullMaskRef;
  if (getImageResource() != NULL && getImageResource()->isInitialized() && !mSceneSuspended)
  {
    if (getImageResource()->getTexture().getPtr() && !getImageResource()->getTexture()->readyForRendering())
    {
      getImageResource()->reloadData();
    }
    else
    {
      context.drawImage(0, 0,
                        getOnscreenWidth(),
                        getOnscreenHeight(),
                        getImageResource()->getTexture(), nullMaskRef,
                        false, NULL, mStretchX, mStretchY, mDownscaleSmooth, mMaskOp);
    }
  }
  // Raise the priority if we're still waiting on the image download    
#if 0
  if (!imageLoaded && getImageResource() != NULL && getImageResource()->isDownloadInProgress())
    getImageResource()->raiseDownloadPriority();
#endif
}
void pxImage::resourceReady(rtString readyResolution)
{
  //rtLogDebug("pxImage::resourceReady(%s) mInitialized=%d for \"%s\"\n",readyResolution.cString(),mInitialized,getImageResource()->getUrl().cString());
  if( !readyResolution.compare("resolve"))
  {
    imageLoaded = true;
    triggerUpdate();
    pxObject::onTextureReady();
    checkStretchX();
    checkStretchY();
    // Now that image is loaded, must force redraw;
    // dimensions could have changed.
    mScene->mDirty = true;
    markDirty();
    pxObject* parent = mParent;
    if( !parent || mResolveWithoutParent)
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

  bool isSceneSuspended = false;
  if (getScene())
  {
    getScene()->suspended(isSceneSuspended);
  }
  mSceneSuspended = isSceneSuspended;
  if (isSceneSuspended && getImageResource())
  {
    getImageResource()->releaseData();
  }
}

void pxImage::resourceDirty()
{
  pxObject::onTextureReady();
}

void pxImage::dispose(bool pumpJavascript)
{
  removeResourceListener();
  mResource = NULL;
  pxObject::dispose(pumpJavascript);
}

void pxImage::checkStretchX()
{
  rtImageResource* imageResource = getImageResource();
  if (mStretchX == pxContextStretch::REPEAT && imageResource != NULL && imageResource->isInitialized())
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
  if (mStretchY == pxContextStretch::REPEAT && imageResource != NULL && imageResource->isInitialized())
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
  mStretchX = (pxContextStretch)v;
  checkStretchX();
  return RT_OK;
}

rtError pxImage::setStretchY(int32_t v)
{
  mStretchY = (pxContextStretch)v;
  checkStretchY();
  return RT_OK;
}

rtError pxImage::flip(bool& v)  const
{
  v = mFlip;
  return RT_OK;
}

rtError pxImage::setFlip(bool v)
{
  mFlip = v;
  rtImageResource* imageResource = getImageResource();
  if (imageResource)
  {
    imageResource->setFlip(v);
  }
  return RT_OK;
}

rtError pxImage::resolveWithoutParent(bool& v)  const
{
  v = mResolveWithoutParent;
  return RT_OK;
}

rtError pxImage::setResolveWithoutParent(bool v)
{
  mResolveWithoutParent = v;
  return RT_OK;
}

rtError pxImage::setMaskOp(int32_t v)
{
  mMaskOp = (pxContextMaskOperation)v;
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

rtError pxImage::texture(uint32_t &v)
{
  v = 0;
  if (getImageResource() != NULL && getImageResource()->isInitialized() && getImageResource()->getTexture().getPtr())
  {
    if (getImageResource()->getTexture()->getNativeId() == 0)
    {
      if (mFlip)
      {
        getImageResource()->getTexture()->setUpsideDown(false);
      }
      getImageResource()->getTexture()->prepareForRendering();
    }
    v = getImageResource()->getTexture()->getNativeId();
  }
  return RT_OK;
}

rtError pxImage::removeResourceListener()
{
  if (mListenerAdded)
  {
    if (getImageResource())
    {
      getImageResource()->removeListener(this);
    }
    mListenerAdded = false;
  }
  return RT_OK;
}

void pxImage::releaseData(bool sceneSuspended)
{
  if (getImageResource())
  {
    getImageResource()->releaseData();
  }
  pxObject::releaseData(sceneSuspended);
}

void pxImage::reloadData(bool sceneSuspended)
{
  if (getImageResource())
  {
    getImageResource()->reloadData();
  }
  pxObject::reloadData(sceneSuspended);
}

uint64_t pxImage::textureMemoryUsage(std::vector<rtObject*> &objectsCounted)
{
  uint64_t textureMemory = 0;
  if (std::find(objectsCounted.begin(), objectsCounted.end(), this) == objectsCounted.end() )
  {
    if (getImageResource())
    {
      textureMemory += getImageResource()->textureMemoryUsage(objectsCounted);
    }
    textureMemory += pxObject::textureMemoryUsage(objectsCounted);
  }
  return textureMemory;
}

rtDefineObject(pxImage,pxObject);
rtDefineProperty(pxImage, url);
rtDefineProperty(pxImage, resource);
rtDefineProperty(pxImage, stretchX);
rtDefineProperty(pxImage, stretchY);
rtDefineProperty(pxImage, flip);
rtDefineProperty(pxImage, resolveWithoutParent);
rtDefineProperty(pxImage, maskOp);
rtDefineProperty(pxImage, downscaleSmooth);
rtDefineMethod(pxImage, texture);
