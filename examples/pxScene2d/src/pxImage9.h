// pxCore CopyRight 2007-2015 John Robinson
// pxImage9.h

#ifndef PX_IMAGE9_H
#define PX_IMAGE9_H

#include "pxOffscreen.h"
#include "pxTextureCacheObject.h"

class pxImage9: public pxObject {
public:
  rtDeclareObject(pxImage9, pxObject);
  rtProperty(url, url, setURL, rtString);
  rtProperty(lInset, lInset, setLInset, float);
  rtProperty(tInset, tInset, setTInset, float);
  rtProperty(rInset, rInset, setRInset, float);
  rtProperty(bInset, bInset, setBInset, float);
  rtReadOnlyProperty(statusCode, statusCode, int32_t);
  rtReadOnlyProperty(httpStatusCode, httpStatusCode, int32_t);

  pxImage9(pxScene2d* scene) : pxObject(scene),ml(0),mt(0),mr(0),mb(0),mTextureCacheObject(), mStatusCode(0), mHttpStatusCode(0) 
  { 
    mTextureCacheObject.setParent(this); 
  }
  
  rtError url(rtString& s) const;
  rtError setURL(const char* s);
  
  rtError lInset(float& v) const { rtValue value;if (getCloneProperty("lInset", value) == RT_OK){ v = value.toFloat(); return RT_OK;}v = ml; return RT_OK; }
  rtError setLInset(float v) { setCloneProperty("lInset",v); /*ml = v;*/ return RT_OK; }
  rtError tInset(float& v) const {rtValue value;if (getCloneProperty("tInset", value) == RT_OK){ v = value.toFloat(); return RT_OK;} v = mt; return RT_OK; }
  rtError setTInset(float v) { setCloneProperty("tInset",v); /*mt = v;*/ return RT_OK; }
  rtError rInset(float& v) const {rtValue value;if (getCloneProperty("rInset", value) == RT_OK){ v = value.toFloat(); return RT_OK;} v = mr; return RT_OK; }
  rtError setRInset(float v) { setCloneProperty("rInset",v); /*mr = v;*/ return RT_OK; }
  rtError bInset(float& v) const {rtValue value;if (getCloneProperty("bInset", value) == RT_OK){ v = value.toFloat(); return RT_OK;} v = mb; return RT_OK; }
  rtError setBInset(float v) { setCloneProperty("bInset",v); /*mb = v;*/ return RT_OK; }

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

  virtual bool onTextureReady(pxTextureCacheObject* textureCacheObject, rtError status);
  virtual void commitClone();

protected:
  virtual void draw();
  void loadImage(rtString url);
  
  rtString mURL;
  float ml, mt, mr, mb;
  pxTextureCacheObject mTextureCacheObject;
  int mStatusCode;
  int mHttpStatusCode;
};

#endif
