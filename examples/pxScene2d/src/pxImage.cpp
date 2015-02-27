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

#include "pxFileDownloader.h"

#include <map>
using namespace std;

extern pxContext context;

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
  mTextureCacheObject.setURL(url);
}

void pxImage::draw() {
  // TODO doing this check here prevents a couple of optimizations
  // need to move this out to some global thread queue... 
  static pxTextureRef nullMaskRef;
  context.drawImage(0, 0, mw, mh, mTexture, nullMaskRef, mXStretch, mYStretch);
}

bool pxImage::onTextureReady(pxTextureCacheObject* textureCacheObject, rtError status)
{
  if (pxObject::onTextureReady(textureCacheObject, status))
  {
    return true;
  }
  else if (textureCacheObject != NULL && status == RT_OK)
  {
    mTexture = textureCacheObject->getTexture();
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
    return true;
  }
  return false;
}

rtDefineObject(pxImage, pxObject);
rtDefineProperty(pxImage, url);
rtDefineProperty(pxImage, xStretch);
rtDefineProperty(pxImage, yStretch);
rtDefineProperty(pxImage, autoSize);

