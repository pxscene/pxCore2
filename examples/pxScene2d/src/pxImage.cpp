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

#include "pxImageDownloader.h"

#include <map>
using namespace std;

extern pxContext context;

typedef map<rtString, pxTextureRef> TextureMap;
TextureMap gTextureCache;


void pxImageDownloadComplete(pxImageDownloadRequest* imageDownloadRequest)
{
  if (imageDownloadRequest != NULL)
  {
      if (imageDownloadRequest->getCallbackData() != NULL)
      {
          pxImage* image = (pxImage*)imageDownloadRequest->getCallbackData();
          image->onImageDownloadComplete(imageDownloadRequest);
          return;
      }
      delete imageDownloadRequest;
  }
}

rtError pxImage::url(rtString& s) const { s = mURL; return RT_OK; }
rtError pxImage::setURL(const char* s) 
{ 
  mURL = s;
  loadImage(mURL);
  return RT_OK;
}

void pxImage::onImageDownloadComplete(pxImageDownloadRequest* imageDownloadRequest)
{
  mImageDownloadMutex.lock();
  mImageDownloadRequest = imageDownloadRequest;
  mImageDownloadIsAvailable = true;
  mImageDownloadMutex.unlock();
}

void pxImage::checkForCompletedImageDownload()
{
    if (mWaitingForImageDownload)
    {
      mImageDownloadMutex.lock();
      if (mImageDownloadIsAvailable)
      {
        if (mImageDownloadRequest != NULL && mImageDownloadRequest->getDownloadStatusCode() == 0)
        {
          pxOffscreen imageOffscreen;
          if (pxLoadImage(mImageDownloadRequest->getDownloadedData(),
                          mImageDownloadRequest->getDownloadedDataSize(), imageOffscreen) != RT_OK)
          {
            rtLogWarn("image load failed"); // TODO: why?
          }
          else
          {
            mTexture = context.createTexture(imageOffscreen);
            rtLogDebug("image %f, %f", mTexture->width(), mTexture->height());
          }
        }
        else
        {
            rtLogWarn("image download failed"); // TODO: why? what happened?

        }
        delete mImageDownloadRequest;
        mImageDownloadRequest = NULL;
        mImageDownloadIsAvailable = false;
        mWaitingForImageDownload = false;
        mw = mTexture->width();
        mh = mTexture->height();
      }
      mImageDownloadMutex.unlock();
    }
}

void pxImage::loadImage(rtString url)
{
  //todo - make case insensitive
  char* s = url.cString();
  const char *result = strstr(s, "http");
  int position = result - s;
  if (position == 0 && strlen(s) > 0)
  {
    pxImageDownloadRequest* downloadRequest = new pxImageDownloadRequest(s, this);
    downloadRequest->setCallbackFunction(pxImageDownloadComplete);
    pxImageDownloader::getInstance()->addToDownloadQueue(downloadRequest);
    
    mWaitingForImageDownload = true;
  }
  else 
  {
    TextureMap::iterator it = gTextureCache.find(url);
    if (it != gTextureCache.end())
    {
      mTexture = it->second;
    }
    else
    {
      rtLogInfo("image texture cache miss");
      pxOffscreen imageOffscreen;
      if (pxLoadImage(s, imageOffscreen) != RT_OK)
        rtLogWarn("image load failed"); // TODO: why?
      else
      {
        mTexture = context.createTexture(imageOffscreen);
        gTextureCache.insert(pair<rtString,pxTextureRef>(s, mTexture));
        rtLogDebug("image %f, %f", mTexture->width(), mTexture->height());
      }
    }
  }

  mw = mTexture->width();
  mh = mTexture->height();
}

void pxImage::draw() {
    
  checkForCompletedImageDownload();
  static pxTextureRef nullMaskRef;
  context.drawImage(0, 0, mw, mh, mTexture, nullMaskRef, mXStretch, mYStretch);
}

rtDefineObject(pxImage, pxObject);
rtDefineProperty(pxImage, url);
rtDefineProperty(pxImage, xStretch);
rtDefineProperty(pxImage, yStretch);

