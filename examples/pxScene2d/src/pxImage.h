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
  rtReadOnlyProperty(statusCode, statusCode, int32_t);
  rtReadOnlyProperty(httpStatusCode, httpStatusCode, int32_t);
  
  pxImage(pxScene2d* scene) : pxObject(scene),mXStretch(PX_NONE),mYStretch(PX_NONE),mTexture(), 
    mTextureCacheObject(), mAutoSize(true), mStatusCode(0), mHttpStatusCode(0),imageLoaded(false)
  { 
    mTextureCacheObject.setParent(this);
  }

  virtual ~pxImage() { rtLogDebug("~pxImage()"); }

  virtual void update(double t) { pxObject::update(t);}
  virtual void onInit();
  virtual void sendPromise();
  virtual void createNewPromise() { rtLogDebug("pxImage ignoring createNewPromise\n"); }
  
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

  rtError statusCode(int32_t& v) const
  {
    v = (int32_t)mStatusCode;
    return RT_OK;
  }

  rtError httpStatusCode(int32_t& v) const
  {
    v = (int32_t)mHttpStatusCode;
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
  int mStatusCode;
  int mHttpStatusCode;
  
  bool imageLoaded;
};

#endif
