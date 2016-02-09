// pxCore CopyRight 2007-2015 John Robinson
// pxImage9.h

#ifndef PX_IMAGE9_H
#define PX_IMAGE9_H

#include "pxOffscreen.h"
//#include "pxTextureCacheObject.h"
#include "rtResource.h"

class pxImage9: public pxObject, rtResourceListener {
public:
  rtDeclareObject(pxImage9, pxObject);
  rtProperty(url, url, setUrl, rtString);
  rtProperty(insetLeft, insetLeft, setInsetLeft, float);
  rtProperty(insetTop, insetTop, setInsetTop, float);
  rtProperty(insetRight, insetRight, setInsetRight, float);
  rtProperty(insetBottom, insetBottom, setInsetBottom, float);
  rtProperty(resource, resource, setResource, rtObjectRef);  

  pxImage9(pxScene2d* scene) : pxObject(scene),mInsetLeft(0),mInsetTop(0),mInsetRight(0),mInsetBottom(0), 
                               imageLoaded(false) 
  { 
    mResource = pxImageManager::getImage("");
  }
  
  rtError url(rtString& s) const;
  rtError setUrl(const char* s);

  rtError resource(rtObjectRef& o) const { /*printf("!!!!!!!!!!!!!!!!!!!!pxImage9 getResource\n");*/o = mResource; return RT_OK; }
  rtError setResource(rtObjectRef o) { /*printf("!!!!!!!!!!!!!!!!!!!!!!!pxImage9 setResource\n");*/mResource = o; return RT_OK; }
    
  rtError insetLeft(float& v) const { v = mInsetLeft; return RT_OK; }
  rtError setInsetLeft(float v) { mInsetLeft = v; return RT_OK; }
  rtError insetTop(float& v) const { v = mInsetTop; return RT_OK; }
  rtError setInsetTop(float v) { mInsetTop = v; return RT_OK; }
  rtError insetRight(float& v) const { v = mInsetRight; return RT_OK; }
  rtError setInsetRight(float v) { mInsetRight = v; return RT_OK; }
  rtError insetBottom(float& v) const { v = mInsetBottom; return RT_OK; }
  rtError setInsetBottom(float v) { mInsetBottom = v; return RT_OK; }


  virtual void onInit();
  virtual void resourceReady(rtString readyResolution);
  //virtual bool onTextureReady(pxTextureCacheObject* textureCacheObject) {return true;}
  virtual void sendPromise();
  virtual void createNewPromise() { rtLogDebug("pxImage9 ignoring createNewPromise\n"); }
  
protected:
  virtual void draw();
  void loadImage(rtString Url);
  inline rtImageResource* getImageResource() const { return (rtImageResource*)mResource.getPtr(); }
  
  float mInsetLeft, mInsetTop, mInsetRight, mInsetBottom;
  rtObjectRef mResource;  
  
  bool imageLoaded;
};

#endif
