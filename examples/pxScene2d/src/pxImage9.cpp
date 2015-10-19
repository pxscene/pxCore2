// pxCore CopyRight 2007-2015 John Robinson
// pxImage9.cpp

#include "rtString.h"
#include "rtRefT.h"
#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxScene2d.h"
#include "pxImage9.h"
#include "pxContext.h"
#include "pxFileDownloader.h"

extern "C"
{
#include "utf8.h"
}

extern pxContext context;

rtError pxImage9::url(rtString& s) const {
  rtValue value;
  if (getCloneProperty("url", value) == RT_OK)
  {
    s = value.toString();
    return RT_OK;
  }
  s = mURL;
  return RT_OK;
}
rtError pxImage9::setURL(const char* s) {
  rtString str(s);
  if (str.length() > 0)
  {
    setCloneProperty("url", str);
  }
  /*mURL = s;
  if (!s || !u8_strlen((char*)s)) 
    return RT_OK;  
  loadImage(mURL);*/
  return RT_OK;
}

void pxImage9::draw() {
  context.drawImage9(mw, mh, ml, mt, mr, mb, mTextureCacheObject.getTexture());
}

void pxImage9::loadImage(rtString url)
{
  mTextureCacheObject.setURL(url);
}

bool pxImage9::onTextureReady(pxTextureCacheObject* textureCacheObject, rtError status)
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

  if (textureCacheObject != NULL && status == RT_OK && textureCacheObject->getTexture().getPtr() != NULL)
  {
    mw = textureCacheObject->getTexture()->width();
    mh = textureCacheObject->getTexture()->height();
    mReady.send("resolve",this);
    return true;
  }
  mReady.send("reject",this);
  return false;
}

void pxImage9::commitClone()
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
        loadImage(mURL);
      }
    }
    else if ((it)->propertyName == "lInset")
    {
      ml = (it)->value.toFloat();
    }
    else if ((it)->propertyName == "tInset")
    {
      mt = (it)->value.toFloat();
    }
    else if ((it)->propertyName == "rInset")
    {
      mr = (it)->value.toFloat();
    }
    else if ((it)->propertyName == "bInset")
    {
      mb = (it)->value.toFloat();
    }
  }
  pxObject::commitClone();
}

rtDefineObject(pxImage9, pxObject);
rtDefineProperty(pxImage9, url);
rtDefineProperty(pxImage9, lInset);
rtDefineProperty(pxImage9, tInset);
rtDefineProperty(pxImage9, rInset);
rtDefineProperty(pxImage9, bInset);
rtDefineProperty(pxImage9,statusCode);
rtDefineProperty(pxImage9,httpStatusCode);
