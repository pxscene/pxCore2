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

extern pxContext context;

rtError pxImage9::url(rtString& s) const { s = mURL; return RT_OK; }
rtError pxImage9::setURL(const char* s) { 
  mURL = s;
  loadImage(mURL);
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

rtDefineObject(pxImage9, pxObject);
rtDefineProperty(pxImage9, url);
rtDefineProperty(pxImage9, lInset);
rtDefineProperty(pxImage9, tInset);
rtDefineProperty(pxImage9, rInset);
rtDefineProperty(pxImage9, bInset);
rtDefineProperty(pxImage9,statusCode);
rtDefineProperty(pxImage9,httpStatusCode);
