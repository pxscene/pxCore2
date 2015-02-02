// pxCore CopyRight 2007-2015 John Robinson
// pxImage.cpp

#include "rtString.h"
#include "rtRefT.h"
#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxScene2d.h"

#include "pxImage.h"

#include "pxContext.h"

#include "pxImageDownloader.h"

extern pxContext context;

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
          if (pxLoadImage(mImageDownloadRequest->getDownloadedData(),
                          mImageDownloadRequest->getDownloadedDataSize(), mOffscreen) != RT_OK)
          {
            rtLogWarn("image load failed"); // TODO: why?
          }
          else
          {
            rtLogDebug("image %d, %d", mOffscreen.width(), mOffscreen.height());
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
        mw = mOffscreen.width();
        mh = mOffscreen.height();
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
    if (pxLoadImage(s, mOffscreen) != RT_OK)
      rtLogWarn("image load failed"); // TODO: why?
    else
      rtLogDebug("image %d, %d", mOffscreen.width(), mOffscreen.height());
  }

  mw = mOffscreen.width();
  mh = mOffscreen.height();
}

void pxImage::draw() {
    
  checkForCompletedImageDownload();
  context.drawImage(mw, mh, mOffscreen, mXStretch, mYStretch);
}

rtDefineObject(pxImage, pxObject);
rtDefineProperty(pxImage, url);
rtDefineProperty(pxImage, xStretch);
rtDefineProperty(pxImage, yStretch);

