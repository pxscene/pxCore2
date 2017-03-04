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

// pxText.h

#include "pxImageA.h"
#include "pxContext.h"

extern pxContext context;

//TODO UGH!!
static pxTextureRef nullMaskRef;

pxImageA::pxImageA(pxScene2d *scene) : pxObject(scene), mStretchX(pxConstantsStretch::NONE), mStretchY(pxConstantsStretch::NONE)
{
  mCurFrame = 0;
  mCachedFrame = UINT32_MAX;
  mFrameTime = -1;
  mPlays = 0;
}

pxImageA::~pxImageA()
{
  gUIThreadQueue.removeAllTasksForObject(this);
}

void pxImageA::onInit() 
{
  mw = mImageWidth;
  mh = mImageHeight;
}

rtError pxImageA::url(rtString &s) const
{
  s = mURL;
  return RT_OK;
}

rtError pxImageA::setUrl(const char *s)
{
  mURL = s;
  pxObject::createNewPromise();

  mCurFrame = 0;
  mCachedFrame = UINT32_MAX;
  mFrameTime = -1;
  mPlays = 0;
  mImageWidth = 0;
  mImageHeight = 0;

  mImageSequence.init();
  if (mURL)
  {
    // Since this object can be released before we get a async completion
    // We need to maintain this object's lifetime
    // TODO review overall flow and organization  
    AddRef();
    mDownloadRequest = new rtFileDownloadRequest(mURL, this);
    mDownloadRequest->setCallbackFunction(pxImageA::onDownloadComplete);
    rtFileDownloader::instance()->addToDownloadQueue(mDownloadRequest);
  }
  else
    mReady.send("resolve", this);

  return RT_OK;
}

void pxImageA::onDownloadComplete(rtFileDownloadRequest* downloadRequest)
{
  pxImageA* image = (pxImageA*)downloadRequest->callbackData();
  if (image) 
  {
    if (downloadRequest->downloadStatusCode() == 0)
    {
      char* data;
      size_t dataSize;
      downloadRequest->downloadedData(data, dataSize);
      pxTimedOffscreenSequence* s = new pxTimedOffscreenSequence;

      if (pxLoadAImage(data, dataSize, *s) == RT_OK)
      {
        gUIThreadQueue.addTask(pxImageA::onDownloadCompleteUI, image, s);
        return;  // Successful and done
      }
      else
        delete s;
    }
    // If we fall through to here we've failed so send NULL to reject promise
    gUIThreadQueue.addTask(pxImageA::onDownloadCompleteUI, image, NULL);
  }
}

void pxImageA::onDownloadCompleteUI(void* context, void* data)
{
  pxImageA* image = (pxImageA*)context;

  if (image)
  {
    if (data)
    {
      pxTimedOffscreenSequence* s = (pxTimedOffscreenSequence*)data;
      image->mImageSequence = *s;
      if (image->mImageSequence.numFrames() > 0)
      {
        pxOffscreen &o = image->mImageSequence.getFrameBuffer(0);
        image->mImageWidth = o.width();
        image->mImageHeight = o.height();
        image->mw = image->mImageWidth;
        image->mh = image->mImageHeight;
      }
      image->mReady.send("resolve", image);
      delete s;
    }
    else
      image->mReady.send("reject", image);

    // Balancing explicit AddRef call
    image->Release();
  }
}

// animation happens here
void pxImageA::update(double t)
{
  pxObject::update(t);

  uint32_t numFrames = mImageSequence.numFrames();

  if (numFrames > 0)
  {
    if (mFrameTime < 0)
    {
      mCurFrame = 0;
      mFrameTime = t;
    }

    for (; mCurFrame < numFrames; mCurFrame++)
    {
      double d = mImageSequence.getDuration(mCurFrame);
      if (mFrameTime + d >= t)
        break;
      mFrameTime += d;
    }

    if (mCurFrame >= numFrames)
    {
      mCurFrame = numFrames - 1; // snap animation to last frame

      if (!mImageSequence.numPlays() || mPlays < mImageSequence.numPlays())
      {
        mFrameTime = -1; // reset animation
        mPlays++;
      }
    }

    if (mCachedFrame != mCurFrame)
    {
      pxOffscreen &o = mImageSequence.getFrameBuffer(mCurFrame);
      mTexture = context.createTexture(o);
      mCachedFrame = mCurFrame;
      pxRect r(0, 0, mImageHeight, mImageWidth);
      mScene->invalidateRect(&r);
    }
  }
}

void pxImageA::draw()
{
  if (mImageSequence.numFrames() > 0)
    context.drawImage(0, 0, mw, mh, mTexture, nullMaskRef, false, NULL, mStretchX, mStretchY);
}

#if 0
void pxImageA::checkStretchX()
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

void pxImageA::checkStretchY()
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

rtDefineObject(pxImageA, pxObject);
rtDefineProperty(pxImageA, url);
//rtDefineProperty(pxImage, resource);
rtDefineProperty(pxImageA, stretchX);
rtDefineProperty(pxImageA, stretchY);
