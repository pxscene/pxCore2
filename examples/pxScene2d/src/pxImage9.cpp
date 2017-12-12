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
  rtImageResource* resourceObj = getImageResource();  
  if(resourceObj != NULL && resourceObj->getUrl().length() > 0 && resourceObj->getUrl().compare(s))
  {
    if(imageLoaded || ((rtPromise*)mReady.getPtr())->status())
    {
      imageLoaded = false;
      pxObject::createNewPromise();
    }
  } 

  mResource = pxImageManager::getImage(s); 
  if(getImageResource() != NULL && getImageResource()->getUrl().length() > 0)
  {
    mListenerAdded = true;
    getImageResource()->addListener(this);
  }
    
  return RT_OK;
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
  if (getImageResource() != NULL)
  {
    context.drawImage9(mw, mh, mInsetLeft, mInsetTop, mInsetRight, mInsetBottom, mSourceLeft, mSourceTop, mSourceRight, mSourceBottom, getImageResource()->getTexture());
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
    if( mw == -1 && getImageResource() != NULL) { mw = getImageResource()->w(); }
    if( mh == -1 && getImageResource() != NULL) { mh = getImageResource()->h(); }
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
rtDefineProperty(pxImage9, sourceLeft);
rtDefineProperty(pxImage9, sourceTop);
rtDefineProperty(pxImage9, sourceRight);
rtDefineProperty(pxImage9, sourceBottom);
rtDefineProperty(pxImage9, resource);
