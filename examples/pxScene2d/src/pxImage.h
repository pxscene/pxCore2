/*

 pxCore Copyright 2005-2018 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// pxImage.h

#ifndef PX_IMAGE_H
#define PX_IMAGE_H

#include "pxContext.h"
#include "rtMutex.h"
#include "pxTexture.h"
//#include "pxTextureCacheObject.h"
#include "pxResource.h"

class pxImage: public pxObject, pxResourceListener 
{
public:
  rtDeclareObject(pxImage, pxObject);
  rtProperty(url, url, setUrl, rtString);
  rtProperty(stretchX, stretchX, setStretchX, int32_t);
  rtProperty(stretchY, stretchY, setStretchY, int32_t);
  rtProperty(maskOp, maskOp, setMaskOp, int32_t);
  
  rtProperty(resource, resource, setResource, rtObjectRef);
  rtProperty(downscaleSmooth, downscaleSmooth, setDownscaleSmooth, bool);
  
  pxImage(pxScene2d* scene) : pxObject(scene),mStretchX(pxConstantsStretch::NONE),mStretchY(pxConstantsStretch::NONE), 
          mMaskOp(pxConstantsMaskOperation::NORMAL), imageLoaded(false), mListenerAdded(false), mDownscaleSmooth(false)
  { 
    mw = -1;
    mh = -1;
    mResource = pxImageManager::getImage("");
  }

  virtual ~pxImage();

  virtual void update(double t) { pxObject::update(t);}
  virtual void onInit();
  virtual void sendPromise();
  virtual void createNewPromise() { rtLogDebug("pxImage ignoring createNewPromise\n"); }
  
  rtError url(rtString& s) const;
  rtError setUrl(const char* s);
  
  rtError stretchX(int32_t& v) const { v = (int32_t)mStretchX; return RT_OK; }
  rtError setStretchX(int32_t v);

  rtError stretchY(int32_t& v) const { v = (int32_t)mStretchY; return RT_OK; }
  rtError setStretchY(int32_t v);
  
  rtError maskOp(int32_t& v)   const { v = (int32_t)  mMaskOp; return RT_OK; }
  rtError setMaskOp(int32_t v);
  
  rtError resource(rtObjectRef& o) const { /*rtLogDebug("!!!!!!!!!!!!!!!!!!!!!!!pxImage getResource\n");*/o = mResource; return RT_OK; }
  rtError setResource(rtObjectRef o);

  rtError downscaleSmooth(bool& v) const;
  rtError setDownscaleSmooth(bool v);

  virtual void resourceReady(rtString readyResolution);
  virtual void resourceDirty();
  //virtual bool onTextureReady(pxTextureCacheObject* textureCacheObject) {return true;}
  // !CLF: To Do: These names are terrible... find better ones!
  virtual float getOnscreenWidth();
  virtual float getOnscreenHeight();
  virtual void dispose(bool pumpJavascript);
  void checkStretchX();
  void checkStretchY();
  rtError removeResourceListener();

  virtual void releaseData(bool sceneSuspended);
  virtual void reloadData(bool sceneSuspended);
  virtual uint64_t textureMemoryUsage();
  
protected:
  virtual void draw();
  void loadImage(rtString Url);
  inline rtImageResource* getImageResource() const { return (rtImageResource*)mResource.getPtr(); }

  pxConstantsStretch::constants mStretchX;
  pxConstantsStretch::constants mStretchY;
  
  pxConstantsMaskOperation::constants  mMaskOp;
  
  rtObjectRef mResource;
  
  bool imageLoaded;
  bool mListenerAdded;
  bool mDownscaleSmooth;
};

#endif
