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

// pxImage9.h

#ifndef PX_IMAGE9_H
#define PX_IMAGE9_H

#include "pxOffscreen.h"
//#include "pxTextureCacheObject.h"
#include "pxResource.h"

class pxImage9: public pxObject, pxResourceListener {
public:
  rtDeclareObject(pxImage9, pxObject);
  rtProperty(url, url, setUrl, rtString);
  rtProperty(insetLeft, insetLeft, setInsetLeft, float);
  rtProperty(insetTop, insetTop, setInsetTop, float);
  rtProperty(insetRight, insetRight, setInsetRight, float);
  rtProperty(insetBottom, insetBottom, setInsetBottom, float);
  rtProperty(resource, resource, setResource, rtObjectRef);  

  pxImage9(pxScene2d* scene) : pxObject(scene),mInsetLeft(0),mInsetTop(0),mInsetRight(0),mInsetBottom(0), 
                               imageLoaded(false), mListenerAdded(false) 
  { 
    mResource = pxImageManager::getImage("");
    mw = -1;
    mh = -1;
  }
  
  rtError url(rtString& s) const;
  rtError setUrl(const char* s);

  rtError resource(rtObjectRef& o) const { /*rtLogDebug("!!!!!!!!!!!!!!!!!!!!pxImage9 getResource\n");*/o = mResource; return RT_OK; }
  rtError setResource(rtObjectRef o);// { /*rtLogDebug("!!!!!!!!!!!!!!!!!!!!!!!pxImage9 setResource\n");*/mResource = o; return RT_OK; }
  rtError removeResourceListener();
    
  rtError insetLeft(float& v) const { v = mInsetLeft; return RT_OK; }
  rtError setInsetLeft(float v) { mInsetLeft = v; return RT_OK; }
  rtError insetTop(float& v) const { v = mInsetTop; return RT_OK; }
  rtError setInsetTop(float v) { mInsetTop = v; return RT_OK; }
  rtError insetRight(float& v) const { v = mInsetRight; return RT_OK; }
  rtError setInsetRight(float v) { mInsetRight = v; return RT_OK; }
  rtError insetBottom(float& v) const { v = mInsetBottom; return RT_OK; }
  rtError setInsetBottom(float v) { mInsetBottom = v; return RT_OK; }


  virtual ~pxImage9();
  virtual void onInit();
  virtual void resourceReady(rtString readyResolution);
  virtual void resourceDirty();
  //virtual bool onTextureReady(pxTextureCacheObject* textureCacheObject) {return true;}
  virtual void sendPromise();
  virtual void createNewPromise() { rtLogDebug("pxImage9 ignoring createNewPromise\n"); }
  virtual float getOnscreenWidth();
  virtual float getOnscreenHeight();

  virtual void releaseData(bool sceneSuspended);
  virtual void reloadData(bool sceneSuspended);
  virtual uint64_t textureMemoryUsage();
  
protected:
  virtual void draw();
  void loadImage(rtString Url);
  inline rtImageResource* getImageResource() const { return (rtImageResource*)mResource.getPtr(); }
  
  float mInsetLeft, mInsetTop, mInsetRight, mInsetBottom;
  rtObjectRef mResource;  
  
  bool imageLoaded;
  bool mListenerAdded;
};

#endif
