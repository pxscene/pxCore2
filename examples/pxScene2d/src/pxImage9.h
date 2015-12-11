// pxCore CopyRight 2007-2015 John Robinson
// pxImage9.h

#ifndef PX_IMAGE9_H
#define PX_IMAGE9_H

#include "pxOffscreen.h"
#include "pxTextureCacheObject.h"

class pxImage9: public pxObject {
public:
  rtDeclareObject(pxImage9, pxObject);
  rtProperty(url, url, setUrl, rtString);
  rtProperty(insetLeft, insetLeft, setInsetLeft, float);
  rtProperty(insetTop, insetTop, setInsetTop, float);
  rtProperty(insetRight, insetRight, setInsetRight, float);
  rtProperty(insetBottom, insetBottom, setInsetBottom, float);
  rtReadOnlyProperty(statusCode, statusCode, int32_t);
  rtReadOnlyProperty(httpStatusCode, httpStatusCode, int32_t);

  pxImage9(pxScene2d* scene) : pxObject(scene),mInsetLeft(0),mInsetTop(0),mInsetRight(0),mInsetBottom(0),mTextureCacheObject(), 
                               mStatusCode(0), mHttpStatusCode(0), imageLoaded(false) 
  { 
    mTextureCacheObject.setParent(this); 
  }
  
  rtError url(rtString& s) const;
  rtError setUrl(const char* s);
  
  rtError insetLeft(float& v) const { v = mInsetLeft; return RT_OK; }
  rtError setInsetLeft(float v) { mInsetLeft = v; return RT_OK; }
  rtError insetTop(float& v) const { v = mInsetTop; return RT_OK; }
  rtError setInsetTop(float v) { mInsetTop = v; return RT_OK; }
  rtError insetRight(float& v) const { v = mInsetRight; return RT_OK; }
  rtError setInsetRight(float v) { mInsetRight = v; return RT_OK; }
  rtError insetBottom(float& v) const { v = mInsetBottom; return RT_OK; }
  rtError setInsetBottom(float v) { mInsetBottom = v; return RT_OK; }

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

  virtual void onInit();
  virtual bool onTextureReady(pxTextureCacheObject* textureCacheObject, rtError status);
  virtual void sendPromise();
  virtual void createNewPromise() { rtLogDebug("pxImage9 ignoring createNewPromise\n"); }
  
protected:
  virtual void draw();
  void loadImage(rtString Url);
  
  rtString mUrl;
  float mInsetLeft, mInsetTop, mInsetRight, mInsetBottom;
  pxTextureCacheObject mTextureCacheObject;
  int mStatusCode;
  int mHttpStatusCode;
  
  bool imageLoaded;
};

#endif
