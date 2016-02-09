// pxCore CopyRight 2007-2015 John Robinson
// pxImage.h

#ifndef PX_IMAGE_H
#define PX_IMAGE_H

#include "pxContext.h"
#include "rtMutex.h"
#include "pxTexture.h"
//#include "pxTextureCacheObject.h"
#include "rtResource.h"

class pxImage: public pxObject, rtResourceListener {
public:
  rtDeclareObject(pxImage, pxObject);
  rtProperty(url, url, setUrl, rtString);
  rtProperty(stretchX, stretchX, setStretchX, int32_t);
  rtProperty(stretchY, stretchY, setStretchY, int32_t);
  rtProperty(resource, resource, setResource, rtObjectRef);
  
  pxImage(pxScene2d* scene) : pxObject(scene),mStretchX(rtConstantsStretch::NONE),mStretchY(rtConstantsStretch::NONE), 
    imageLoaded(false)
  { 
    mw = -1;
    mh = -1;
    mResource = pxImageManager::getImage("");
  }

  virtual ~pxImage() { rtLogDebug("~pxImage()"); }

  virtual void update(double t) { pxObject::update(t);}
  virtual void onInit();
  virtual void sendPromise();
  virtual void createNewPromise() { rtLogDebug("pxImage ignoring createNewPromise\n"); }
  
  rtError url(rtString& s) const;
  rtError setUrl(const char* s);
  
  rtError stretchX(int32_t& v) const { v = (int32_t)mStretchX; return RT_OK; }
  rtError setStretchX(int32_t v)
  {
    mStretchX = (rtConstantsStretch::constants)v;
    return RT_OK;
  }

  rtError stretchY(int32_t& v) const { v = (int32_t)mStretchY; return RT_OK; }
  rtError setStretchY(int32_t v)
  {
    mStretchY = (rtConstantsStretch::constants)v;
    return RT_OK;
  }
  
  rtError resource(rtObjectRef& o) const { /*printf("!!!!!!!!!!!!!!!!!!!!!!!pxImage getResource\n");*/o = mResource; return RT_OK; }
  rtError setResource(rtObjectRef o);

  virtual void resourceReady(rtString readyResolution);
  //virtual bool onTextureReady(pxTextureCacheObject* textureCacheObject) {return true;}
  // !CLF: To Do: These names are terrible... find better ones!
  virtual float getOnscreenWidth();
  virtual float getOnscreenHeight();
  
protected:
  virtual void draw();
  void loadImage(rtString Url);
  inline rtImageResource* getImageResource() const { return (rtImageResource*)mResource.getPtr(); }

  rtConstantsStretch::constants mStretchX;
  rtConstantsStretch::constants mStretchY;
  rtObjectRef mResource;
  
  bool imageLoaded;
};

#endif
