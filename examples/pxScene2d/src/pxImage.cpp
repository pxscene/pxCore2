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
extern int gImageDownloadsPending;

typedef map<rtString, pxTextureRef> TextureMap;
TextureMap gTextureCache;


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

void pxImage::loadImage(rtString url)
{
  TextureMap::iterator it = gTextureCache.find(url);
  if (it != gTextureCache.end())
  {
    mTexture = it->second;
    if (mAutoSize && mTexture.getPtr() != NULL)
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
      pxImageDownloadRequest* downloadRequest = 
        new pxImageDownloadRequest(s, this);
      gImageDownloadsPending++;
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
  static pxTextureRef nullMaskRef;
  context.drawImage(0, 0, mw, mh, mTexture, nullMaskRef, mXStretch, mYStretch);
}

void pxImage::setTexture(pxTextureRef texture)
{
  // TODO... tried to access url from mImageDownloadRequest and
  // it seemed to be coming back NULL.. switched to using mURL for now
  mTexture = texture;
  gTextureCache.insert(pair<rtString,pxTextureRef>(mURL.cString(), 
                                                   mTexture));
  rtLogDebug("image %f, %f", mTexture->width(), mTexture->height());
  if (mAutoSize && mTexture.getPtr() != NULL)
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

rtDefineObject(pxImage, pxObject);
rtDefineProperty(pxImage, url);
rtDefineProperty(pxImage, xStretch);
rtDefineProperty(pxImage, yStretch);
rtDefineProperty(pxImage, autoSize);

