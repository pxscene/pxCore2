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

extern "C"
{
#include "utf8.h"
}

#include <map>
using namespace std;

extern pxContext context;

void pxImage::onInit()
{
  setURL(mURL);
}

rtError pxImage::url(rtString& s) const {
  rtValue value;
  if (getCloneProperty("url", value) == RT_OK)
  {
    s = value.toString();
    return RT_OK;
  }
  s = mURL;
  return RT_OK;
}

rtError pxImage::setURL(const char* s) 
{
  rtString str(s);
  if (str.length() > 0)
  {
    setCloneProperty("url", str);
    printf("setting url: %s\n", str.cString());
  }

  /*mURL = s;
  if (!s || !u8_strlen((char*)s)) 
    return RT_OK;
  if (mInitialized)
    loadImage(mURL);
  else
    rtLogDebug("Deferring image load until pxImage is initialized.");*/

  return RT_OK;
}

void pxImage::loadImage(rtString url)
{
  mTextureCacheObject.setURL(url);
}

void pxImage::draw() {
  static pxTextureRef nullMaskRef;
  context.drawImage(0, 0, mw, mh, mTexture, nullMaskRef, 
                    mXStretch, mYStretch);
  if (mTextureCacheObject.isDownloadInProgress())
  {
    mTextureCacheObject.raiseDownloadPriority();
  }
}

bool pxImage::onTextureReady(pxTextureCacheObject* textureCacheObject, rtError status)
{
  if (pxObject::onTextureReady(textureCacheObject, status))
  {
    mReady.send("resolve",this);
    return true;
  }

  if (textureCacheObject != NULL)
  {
    mStatusCode = textureCacheObject->getStatusCode();
    mHttpStatusCode = textureCacheObject->getHttpStatusCode();
  }
  if (textureCacheObject != NULL && status == RT_OK)
  {
    mReady.send("resolve",this);
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
  mReady.send("reject",this);
  return false;
}

void pxImage::commitClone()
{
  const vector<pxObjectCloneProperty>& properties = mClone->getProperties();
  for(vector<pxObjectCloneProperty>::const_iterator it = properties.begin();
      it != properties.end(); ++it)
  {
    if ((it)->propertyName == "url")
    {
      mURL = (it)->value.toString();
      if (mURL.length() > 0)
      {
        if (mInitialized)
        {
          loadImage(mURL);
        }
        else
          rtLogDebug("Deferring image load until pxImage is initialized.");
      }
    }
    else if ((it)->propertyName == "xStretch")
    {
      mXStretch = (pxStretch)(it)->value.toInt32();
    }
    else if ((it)->propertyName == "yStretch")
    {
      mYStretch = (pxStretch)(it)->value.toInt32();
    }
    else if ((it)->propertyName == "autoSize")
    {
      mAutoSize = (it)->value.toBool();
    }
  }
  pxObject::commitClone();
}

rtDefineObject(pxImage,pxObject);
rtDefineProperty(pxImage,url);
rtDefineProperty(pxImage,xStretch);
rtDefineProperty(pxImage,yStretch);
rtDefineProperty(pxImage,autoSize);
rtDefineProperty(pxImage,statusCode);
rtDefineProperty(pxImage,httpStatusCode);


