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

void pxImage::onInit()
{
  setURL(mURL);
}

rtError pxImage::url(rtString& s) const { s = mURL; return RT_OK; }
rtError pxImage::setURL(const char* s) 
{ 
  mURL = s;
  if (!s) 
    return RT_OK;
  if (mInitialized)
    loadImage(mURL);
  else
    rtLogDebug("Deferring image load until pxImage is initialized.");
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
        
        if (mImageDownloadRequest != NULL && 
            mImageDownloadRequest->getDownloadStatusCode() == 0)
        {
          pxOffscreen imageOffscreen;
          if (pxLoadImage(mImageDownloadRequest->getDownloadedData(),
                          mImageDownloadRequest->getDownloadedDataSize(), 
                          imageOffscreen) != RT_OK)
          {
            rtLogWarn("image load failed"); // TODO: why?
          }
          else
          {
            mTexture = context.createTexture(imageOffscreen);
	    // TODO... tried to access url from mImageDownloadRequest and
	    // it seemed to be coming back NULL.. switched to using mURL for now
            gTextureCache.insert(pair<rtString,pxTextureRef>(mURL.cString(), 
                                                             mTexture));
            rtLogDebug("image %f, %f", mTexture->width(), mTexture->height());
          }
        }
        else
          rtLogWarn("image download failed"); // TODO: why? what happened?

        delete mImageDownloadRequest;
        mImageDownloadRequest = NULL;
        mImageDownloadIsAvailable = false;
        mWaitingForImageDownload = false;
        if (mAutoSize)
        {
          mw = mTexture->width();
          mh = mTexture->height();
        }
        // send after width and height have been set
	rtObjectRef e = new rtMapObject;
	e.set("name", "onReady");
	e.set("target", this);
        mEmit.send("onReady", e);
      }
      mImageDownloadMutex.unlock();
    }
}

void pxImage::loadImage(rtString url)
{
  TextureMap::iterator it = gTextureCache.find(url);
  if (it != gTextureCache.end())
  {
    mTexture = it->second;
    if (mAutoSize)
    {
      mw = mTexture->width();
      mh = mTexture->height();
    }
    rtObjectRef e = new rtMapObject;
    e.set("name", "onReady");
    e.set("target", this);
    mEmit.send("onReady", e);
  }
  else
  {
    rtLogInfo("Image texture cache miss");
    char* s = url.cString();
    const char *result = strstr(s, "http");
    int position = result - s;
    if (position == 0 && strlen(s) > 0)
    {
      mWaitingForImageDownload = true;
      pxImageDownloadRequest* downloadRequest = 
        new pxImageDownloadRequest(s, this);
      downloadRequest->setCallbackFunction(pxImageDownloadComplete);
      pxImageDownloader::getInstance()->addToDownloadQueue(downloadRequest);
    }
    else 
    {
      pxOffscreen imageOffscreen;
      if (pxLoadImage(s, imageOffscreen) != RT_OK)
        rtLogWarn("image load failed"); // TODO: why?
      else
      {
        mTexture = context.createTexture(imageOffscreen);
        gTextureCache.insert(pair<rtString,pxTextureRef>(s, mTexture));
        rtLogDebug("image %f, %f", mTexture->width(), mTexture->height());
      }
      if (mAutoSize)
      {
        mw = mTexture->width();
        mh = mTexture->height();
      }
      rtObjectRef e = new rtMapObject;
      e.set("name", "onReady");
      e.set("target", this);
      mEmit.send("onReady", e);
    }
  }
}

void pxImage::draw() {
  // TODO doing this check here prevents a couple of optimizations
  // need to move this out to some global thread queue... 
  checkForCompletedImageDownload();
  static pxTextureRef nullMaskRef;
  context.drawImage(0, 0, mw, mh, mTexture, nullMaskRef, mXStretch, mYStretch);
}

rtDefineObject(pxImage, pxObject);
rtDefineProperty(pxImage, url);
rtDefineProperty(pxImage, xStretch);
rtDefineProperty(pxImage, yStretch);
rtDefineProperty(pxImage, autoSize);

