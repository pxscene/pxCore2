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

// pxText.h

#include "pxImageA.h"
#include "pxContext.h"

extern pxContext context;

//TODO UGH!!
static pxTextureRef nullMaskRef;

pxImageA::pxImageA(pxScene2d *scene) : pxObject(scene), 
                                       mImageWidth(0), mImageHeight(0),
                                       mStretchX(pxConstantsStretch::NONE), mStretchY(pxConstantsStretch::NONE),
                                       mResource(), mImageLoaded(false), mListenerAdded(false)
{
  mCurFrame = 0;
  mCachedFrame = UINT32_MAX;
  mFrameTime = -1;
  mPlays = 0;
  mResource = pxImageManager::getImageA("");
}

pxImageA::~pxImageA()
{
  removeResourceListener();
  mResource = NULL;
}

void pxImageA::onInit() 
{
  mw = static_cast<float>(mImageWidth);
  mh = static_cast<float>(mImageHeight);
}

rtError pxImageA::url(rtString &s) const
{
  if (getImageAResource() != NULL)
  {
    s = getImageAResource()->getUrl();
  }
  else
  {
    s = "";
  }
  return RT_OK;
}

rtError pxImageA::setUrl(const char *s)
{
#ifdef ENABLE_PERMISSIONS_CHECK
  if (mScene != NULL && RT_OK != mScene->permissions()->allows(s, rtPermissions::DEFAULT))
    return RT_ERROR_NOT_ALLOWED;
#endif

  rtImageAResource* resourceObj = getImageAResource();
  if( resourceObj != NULL && resourceObj->getUrl().length() > 0 && resourceObj->getUrl().compare(s))
  {
    if(mImageLoaded || ((rtPromise*)mReady.getPtr())->status())
    {
      mCurFrame = 0;
      mCachedFrame = UINT32_MAX;
      mFrameTime = -1;
      mPlays = 0;
      mImageWidth = 0;
      mImageHeight = 0;
      mImageLoaded = false;
      pxObject::createNewPromise();
    }
  }
  removeResourceListener();
  mResource = pxImageManager::getImageA(s, NULL, mScene ? mScene->cors() : NULL, mScene ? mScene->getArchive(): NULL);

  if(getImageAResource() != NULL && getImageAResource()->getUrl().length() > 0 && !mImageLoaded) {
    mListenerAdded = true;
    getImageAResource()->addListener(this);
  }

  return RT_OK;
}

// animation happens here
void pxImageA::update(double t)
{
  pxObject::update(t);

  if (getImageAResource() == NULL || !mImageLoaded)
  {
    return;
  }

  pxTimedOffscreenSequence& imageSequence = getImageAResource()->getTimedOffscreenSequence();
  uint32_t numFrames = imageSequence.numFrames();

  if (numFrames > 0)
  {
    if (mFrameTime < 0)
    {
      mCurFrame = 0;
      mFrameTime = t;
    }

    for (; mCurFrame < numFrames; mCurFrame++)
    {
      double d = imageSequence.getDuration(mCurFrame);
      if (mFrameTime + d >= t)
        break;
      mFrameTime += d;
    }

    if (mCurFrame >= numFrames)
    {
      mCurFrame = numFrames - 1; // snap animation to last frame

      if (!imageSequence.numPlays() || mPlays < imageSequence.numPlays())
      {
        mFrameTime = -1; // reset animation
        mPlays++;
      }
    }

    if (mCachedFrame != mCurFrame)
    {
      pxOffscreen &o = imageSequence.getFrameBuffer(mCurFrame);
      mTexture = context.createTexture(o);
      mCachedFrame = mCurFrame;
      pxRect r(0, 0, mImageHeight, mImageWidth);
      mScene->invalidateRect(&r);
    }
  }
}

void pxImageA::draw()
{
  if (getImageAResource() != NULL && mImageLoaded && !mSceneSuspended)
  {
    pxTimedOffscreenSequence &imageSequence = getImageAResource()->getTimedOffscreenSequence();
    if (imageSequence.numFrames() > 0)
    {
      context.drawImage(0, 0, mw, mh, mTexture, nullMaskRef, false, NULL, mStretchX, mStretchY);
    }
  }
}

void pxImageA::dispose(bool pumpJavascript)
{
  if (mListenerAdded)
  {
    if (getImageAResource())
    {
      getImageAResource()->removeListener(this);
    }
    mResource = NULL;
    mListenerAdded = false;
  }
  pxObject::dispose(pumpJavascript);
}

#if 0
void pxImageA::checkStretchX()
{
  rtImageResource* imageResource = getImageAResource();
  if (mStretchX == pxConstantsStretch::REPEAT && imageResource != NULL)
  {
    pxTextureRef texture = imageResource->getTexture();
    if (texture.getPtr() != NULL && (!isPowerOfTwo(texture->width()) || !isPowerOfTwo(texture->height())))
    {
      rtLogWarn("stretchX set to REPEAT but image is not a power of 2 for image: %s", imageResource->getUrl().cString());
    }
  }
}

void pxImageA::checkStretchY()
{
  rtImageResource* imageResource = getImageAResource();
  if (mStretchY == pxConstantsStretch::REPEAT && imageResource != NULL)
  {
    pxTextureRef texture = imageResource->getTexture();
    if (texture.getPtr() != NULL && (!isPowerOfTwo(texture->width()) || !isPowerOfTwo(texture->height())))
    {
      rtLogWarn("stretchY set to REPEAT but image is not a power of 2 for image: %s", imageResource->getUrl().cString());
    }
  }
}
#endif

rtError pxImageA::setStretchX(int32_t v)
{
  mStretchX = (pxConstantsStretch::constants)v;
  //checkStretchX();
  return RT_OK;
}

rtError pxImageA::setStretchY(int32_t v)
{
  mStretchY = (pxConstantsStretch::constants)v;
  //checkStretchY();
  return RT_OK;
}

rtError pxImageA::setResource(rtObjectRef o)
{
  if(!o)
  {
    setUrl("");
    return RT_OK;
  }

  rtString desc;
  o.sendReturns("description",desc);
  if(!desc.compare("rtImageAResource"))
  {
    rtString url;
    url = o.get<rtString>("url");
    // Only create new promise if url is different
    if( getImageAResource() != NULL && getImageAResource()->getUrl().compare(o.get<rtString>("url")) )
    {
      removeResourceListener();
      mResource = o;
      mImageLoaded = false;
      pxObject::createNewPromise();
      mListenerAdded = true;
      getImageAResource()->addListener(this);
    }
    return RT_OK;
  }
  else
  {
    rtLogError("Object passed as resource is not an imageAResource!\n");
    pxObject::onTextureReady();
    mReady.send("reject",this);
    return RT_ERROR;
  }

}

void pxImageA::loadImageSequence()
{
  if (getImageAResource() != NULL && getImageAResource()->getLoadStatus("statusCode") == 0)
  {
    pxTimedOffscreenSequence& imageSequence = getImageAResource()->getTimedOffscreenSequence();
    if (imageSequence.numFrames() > 0)
    {
      pxOffscreen &o = imageSequence.getFrameBuffer(0);
      mImageWidth = o.width();
      mImageHeight = o.height();
      mw = static_cast<float>(mImageWidth);
      mh = static_cast<float>(mImageHeight);
    }
    mReady.send("resolve", this);
  }
  else
  {
    mReady.send("reject", this);
  }
}

void pxImageA::resourceReady(rtString readyResolution)
{
#if 0
  checkStretchX();
  checkStretchY();
#endif //0
  if( !readyResolution.compare("resolve"))
  {
    mImageLoaded = true;
    pxObject::onTextureReady();
    mScene->mDirty = true;
    loadImageSequence();
    pxObject* parent = mParent;
    if( !parent)
    {
      // Send the promise here because the image will not get an 
      // update call until it has a parent
      sendPromise();
    }
  }
  else
  {
    pxObject::onTextureReady();
    mReady.send("reject",this);
  }
}

void pxImageA::resourceDirty()
{
  pxObject::onTextureReady();
}

rtError pxImageA::removeResourceListener()
{
  if (mListenerAdded)
  {
    if (getImageAResource())
    {
      getImageAResource()->removeListener(this);
    }
    mListenerAdded = false;
  }
  return RT_OK;
}

void pxImageA::releaseData(bool sceneSuspended)
{
  pxObject::releaseData(sceneSuspended);
}

void pxImageA::reloadData(bool sceneSuspended)
{
  pxObject::reloadData(sceneSuspended);
}

uint64_t pxImageA::textureMemoryUsage()
{
  uint64_t textureMemory = 0;
  if (mTexture.getPtr() != NULL)
  {
    textureMemory += (mTexture->width() * mTexture->height() * 4);
  }
  textureMemory += pxObject::textureMemoryUsage();
  return textureMemory;
}

rtDefineObject(pxImageA, pxObject);
rtDefineProperty(pxImageA, url);
rtDefineProperty(pxImageA, resource);
rtDefineProperty(pxImageA, stretchX);
rtDefineProperty(pxImageA, stretchY);
