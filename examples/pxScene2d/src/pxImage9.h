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
  rtProperty(x1, x1, setX1, float);
  rtProperty(y1, y1, setY1, float);
  rtProperty(x2, x2, setX2, float);
  rtProperty(y2, y2, setY2, float);
  rtReadOnlyProperty(statusCode, statusCode, int32_t);
  rtReadOnlyProperty(httpStatusCode, httpStatusCode, int32_t);

  pxImage9(pxScene2d* scene) : pxObject(scene),mx1(0),my1(0),mx2(0),my2(0),mTextureCacheObject(), mStatusCode(0), mHttpStatusCode(0) 
  { 
    mTextureCacheObject.setParent(this); 
  }
  
  rtError url(rtString& s) const;
  rtError setURL(const char* s);

  rtError x1(float& v) const { v = mx1; return RT_OK; }
  rtError setX1(float v) { mx1 = v; return RT_OK; }
  rtError y1(float& v) const { v = my1; return RT_OK; }
  rtError setY1(float v) { my1 = v; return RT_OK; }
  rtError x2(float& v) const { v = mx2; return RT_OK; }
  rtError setX2(float v) { mx2 = v; return RT_OK; }
  rtError y2(float& v) const { v = my2; return RT_OK; }
  rtError setY2(float v) { my2 = v; return RT_OK; }

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

protected:
  virtual void draw();
  void loadImage(rtString url);
  
  rtString mURL;
  float mx1, my1, mx2, my2;
  pxTextureCacheObject mTextureCacheObject;
  int mStatusCode;
  int mHttpStatusCode;
};

#endif
