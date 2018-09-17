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

// pxImageA.h

#ifndef PX_IMAGEA_H
#define PX_IMAGEA_H

#include "pxScene2d.h"

#include "pxUtil.h"
#include "pxResource.h"

class pxImageA: public pxObject, pxResourceListener
{
public:
  rtDeclareObject(pxImageA, pxObject);
  rtProperty(url, url, setUrl, rtString);
  rtProperty(stretchX, stretchX, setStretchX, int32_t);
  rtProperty(stretchY, stretchY, setStretchY, int32_t);
  rtProperty(resource, resource, setResource, rtObjectRef);

  pxImageA(pxScene2d* scene);
  virtual ~pxImageA();
  
  rtError url(rtString& s) const;
  rtError setUrl(const char* s);

  rtError stretchX(int32_t& v) const { v = (int32_t)mStretchX; return RT_OK; }
  rtError setStretchX(int32_t v);

  rtError stretchY(int32_t& v) const { v = (int32_t)mStretchY; return RT_OK; }
  rtError setStretchY(int32_t v);

  rtError resource(rtObjectRef& o) const { o = mResource; return RT_OK; }
  rtError setResource(rtObjectRef o);
  rtError removeResourceListener();
  virtual void resourceReady(rtString readyResolution);
  virtual void resourceDirty();
  virtual void createNewPromise() { rtLogDebug("pxImageA ignoring createNewPromise\n"); }

  virtual void update(double t);
  virtual void draw();
  virtual void dispose(bool pumpJavascript);

  virtual void releaseData(bool sceneSuspended);
  virtual void reloadData(bool sceneSuspended);
  virtual uint64_t textureMemoryUsage();
  
protected:
  virtual void onInit();
  inline rtImageAResource* getImageAResource() const { return (rtImageAResource*)mResource.getPtr(); }

  void sendPromise() {} // shortcircuit  TODO...not sure if I like this pattern
  void loadImageSequence();

  uint32_t mCurFrame;
  uint32_t mCachedFrame;
  uint32_t mPlays;

  uint32_t mImageWidth;
  uint32_t mImageHeight;

  pxTextureRef mTexture;

  double mFrameTime;
  pxConstantsStretch::constants mStretchX;
  pxConstantsStretch::constants mStretchY;

  rtObjectRef mResource;
  bool mImageLoaded;
  bool mListenerAdded;
};

#endif
