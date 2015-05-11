// pxCore CopyRight 2007-2015 John Robinson
// pxImage.h

#ifndef PX_IMAGE_H
#define PX_IMAGE_H

#include "pxContext.h"
#include "rtMutex.h"
#include "pxTexture.h"
#include "pxTextureCacheObject.h"

class pxImage: public pxObject {
public:
  rtDeclareObject(pxImage, pxObject);
  rtProperty(url, url, setURL, rtString);
  rtProperty(xStretch, xStretch, setXStretch, int32_t);
  rtProperty(yStretch, yStretch, setYStretch, int32_t);
  rtProperty(autoSize, autoSize, setAutoSize, bool);
  rtReadOnlyProperty(ready, ready, rtObjectRef);
  
  pxImage(pxScene2d* scene) : pxObject(scene),mXStretch(PX_NONE),mYStretch(PX_NONE),mTexture(), 
    mTextureCacheObject(), mAutoSize(true) 
  { 
    mTextureCacheObject.setParent(this);
    mReady = new rtPromise;
  }

  virtual ~pxImage() { rtLogInfo("~pxImage()"); }

  virtual void onInit();
  
  rtError url(rtString& s) const;
  rtError setURL(const char* s);
  
  rtError xStretch(int32_t& v) const { v = (int32_t)mXStretch; return RT_OK; }
  rtError setXStretch(int32_t v)
  {
    mXStretch = (pxStretch)v;
    return RT_OK;
  }

  rtError yStretch(int32_t& v) const { v = (int32_t)mYStretch; return RT_OK; }
  rtError setYStretch(int32_t v)
  {
    mYStretch = (pxStretch)v;
    return RT_OK;
  }

  rtError autoSize(bool& v) const
  {
    v = mAutoSize;
    return RT_OK;
  }

  rtError setAutoSize(bool v)
  {
    mAutoSize = v;
    return RT_OK;
  }

  rtError ready(rtObjectRef& v) const
  {
    v = mReady;
    return RT_OK;
  }
  
  pxTextureRef getTexture()
  {
    return mTexture;
  }

  virtual bool onTextureReady(pxTextureCacheObject* textureCacheObject, rtError status);
  
protected:
  virtual void draw();
  void loadImage(rtString url);
  
  rtString mURL;
  pxStretch mXStretch;
  pxStretch mYStretch;
  pxTextureRef mTexture;
  pxTextureCacheObject mTextureCacheObject;
  bool mAutoSize;
  rtObjectRef mReady;
};

#endif
