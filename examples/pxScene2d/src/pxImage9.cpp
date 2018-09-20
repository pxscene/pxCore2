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

// pxImage9.cpp

#include "rtString.h"
#include "rtRef.h"
#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxScene2d.h"
#include "pxImage9.h"
#include "pxContext.h"
#include "rtFileDownloader.h"

extern pxContext context;

pxImage9::~pxImage9()
{
  rtLogDebug("~pxImage9()");
  removeResourceListener();
  mResource = NULL;
}

void pxImage9::onInit()
{
  mInitialized = true;
  if (getImageResource() != NULL)
  {
    setUrl(getImageResource()->getUrl());
  }
  else
  {
    setUrl("");
  }
}

rtError pxImage9::url(rtString& s) const 
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

rtError pxImage9::setUrl(const char* s) 
{
#ifdef ENABLE_PERMISSIONS_CHECK
  if (mScene != NULL && RT_OK != mScene->permissions()->allows(s, rtPermissions::DEFAULT))
    return RT_ERROR_NOT_ALLOWED;
#endif

  rtImageResource* resourceObj = getImageResource();  
  if(resourceObj != NULL && resourceObj->getUrl().length() > 0 && resourceObj->getUrl().compare(s))
  {
    if(imageLoaded || ((rtPromise*)mReady.getPtr())->status())
    {
      imageLoaded = false;
      pxObject::createNewPromise();
    }
  }

  removeResourceListener();
  mResource = pxImageManager::getImage(s, NULL, mScene ? mScene->cors() : NULL, 0, 0, 1.0f, 1.0f, mScene ? mScene->getArchive(): NULL);
  if(getImageResource() != NULL && (getImageResource()->getUrl().length() > 0) && mInitialized && !imageLoaded)
  {
    mListenerAdded = true;
    getImageResource()->addListener(this);
  }
    
  return RT_OK;
}

/**
 * setResource
 * */
rtError pxImage9::setResource(rtObjectRef o) 
{ 
    
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
      pxObject::createNewPromise();
      mListenerAdded = true;
      getImageResource()->addListener(this);
    }
    return RT_OK; 
  } 
  else 
  {
    rtLogError("Object passed as resource is not an imageResource!\n");
    pxObject::onTextureReady();
    // Call createNewPromise to ensure the old promise hadn't already been resolved
    pxObject::createNewPromise();
    mReady.send("reject",this);
    return RT_ERROR; 
  }

}

void pxImage9::sendPromise() 
{ 
  //rtLogDebug("image9 init=%d imageLoaded=%d\n",mInitialized,imageLoaded);
  if(mInitialized && imageLoaded && !((rtPromise*)mReady.getPtr())->status()) 
  {
    if (getImageResource() != NULL)
    {
      rtLogDebug("pxImage9 SENDPROMISE for %s\n", getImageResource()->getUrl().cString());
    }
    mReady.send("resolve",this);
  } 
}

float pxImage9::getOnscreenWidth() 
{
  return mw;

}
float pxImage9::getOnscreenHeight() 
{ 
  return mh;
}


void pxImage9::draw() {
  if (getImageResource() != NULL && getImageResource()->isInitialized() && !mSceneSuspended)
  {
    context.drawImage9(mw, mh, mInsetLeft, mInsetTop, mInsetRight, mInsetBottom, getImageResource()->getTexture());
  }
}

void pxImage9::resourceReady(rtString readyResolution)
{
  //rtLogDebug("pxImage9::resourceReady()\n");
  if( !readyResolution.compare("resolve"))
  {
    imageLoaded = true; 
    // nineslice gets its w and h from the image only if
    // not set for the pxImage9
    if( mw == -1 && getImageResource() != NULL) { mw = static_cast<float>(getImageResource()->w()); }
    if( mh == -1 && getImageResource() != NULL) { mh = static_cast<float>(getImageResource()->h()); }
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

void pxImage9::resourceDirty()
{
  pxObject::onTextureReady();
}

rtError pxImage9::removeResourceListener()
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

void pxImage9::releaseData(bool sceneSuspended)
{
  if (getImageResource())
  {
    getImageResource()->releaseData();
  }
  pxObject::releaseData(sceneSuspended);
}

void pxImage9::reloadData(bool sceneSuspended)
{
  if (getImageResource())
  {
    getImageResource()->reloadData();
  }
  pxObject::reloadData(sceneSuspended);
}

uint64_t pxImage9::textureMemoryUsage()
{
  uint64_t textureMemory = 0;
  if (getImageResource())
  {
    textureMemory += getImageResource()->textureMemoryUsage();
  }
  textureMemory += pxObject::textureMemoryUsage();
  return textureMemory;
}

rtDefineObject(pxImage9, pxObject);
rtDefineProperty(pxImage9, url);
rtDefineProperty(pxImage9, insetLeft);
rtDefineProperty(pxImage9, insetTop);
rtDefineProperty(pxImage9, insetRight);
rtDefineProperty(pxImage9, insetBottom);
rtDefineProperty(pxImage9, resource);
