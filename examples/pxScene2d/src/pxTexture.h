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

#ifndef PX_TEXTURE_H
#define PX_TEXTURE_H

#include "pxCore.h"
#include "rtRef.h"
#include "pxOffscreen.h"
#include "rtAtomic.h"

enum pxTextureType { 
  PX_TEXTURE_UNKNOWN = 0,
  PX_TEXTURE_OFFSCREEN = 1, 
  PX_TEXTURE_ALPHA = 2, 
  PX_TEXTURE_NATIVE = 3,
  PX_TEXTURE_FRAME_BUFFER = 4
};

class pxTexture;

class pxTextureNative
{
public:
  virtual pxError bindGLTexture(int tLoc) = 0;
  virtual pxError bindGLTextureAsMask(int mLoc) = 0;
};

class pxTextureListener
{
public:
  virtual void textureReady() = 0;
};

class pxTexture: public pxTextureNative
{
public:
  pxTexture() : mRef(0), mTextureType(PX_TEXTURE_UNKNOWN), mPremultipliedAlpha(false), mLastRenderTick(0),
                mDownscaleSmooth(false)
  { }
  virtual ~pxTexture() {}

  virtual unsigned long AddRef()
  {
    return rtAtomicInc(&mRef);
  }

  virtual unsigned long Release()
  {
    unsigned long l = rtAtomicDec(&mRef);
    if (l == 0)
      delete this;
    return l;
  }
  virtual pxError updateTexture(int /*x*/, int /*y*/, int /*w*/, int /*h*/,  void* /*buffer*/) { return PX_FAIL; }
  virtual pxError bindTexture() { return PX_FAIL; }
  virtual pxError bindTextureAsMask() { return PX_FAIL; }
  virtual pxError createTexture(pxOffscreen&) { return PX_FAIL; }
  virtual pxError deleteTexture() = 0;
  virtual int width() = 0;
  virtual int height() = 0;
  virtual pxError resizeTexture(int w, int h) { (void)w; (void)h; return PX_FAIL; }
  virtual pxError getOffscreen(pxOffscreen& o) = 0;
  virtual unsigned int getNativeId() { return 0; }
  pxTextureType getType() { return mTextureType; }
  virtual pxError prepareForRendering() { return PX_OK; }
  virtual pxError loadTextureData() { return PX_OK; }
  virtual pxError unloadTextureData() { return PX_OK; }
  virtual pxError freeOffscreenData() { return PX_OK; }
  virtual pxError setTextureListener(pxTextureListener* /*textureListener*/) { return PX_OK; }
  bool premultipliedAlpha() { return mPremultipliedAlpha; }
  void enablePremultipliedAlpha(bool enable) { mPremultipliedAlpha = enable; }
  virtual void* getSurface() { return NULL; }
  uint32_t lastRenderTick() { return mLastRenderTick; }
  void setLastRenderTick(uint32_t renderTick) { mLastRenderTick = renderTick; }
  void setDownscaleSmooth(bool downscaleSmooth) { mDownscaleSmooth = downscaleSmooth; }
  bool downscaleSmooth() { return mDownscaleSmooth; }
  bool initialized() { return true; }
protected:
  rtAtomic mRef;
  pxTextureType mTextureType;
  bool mPremultipliedAlpha;
  uint32_t mLastRenderTick;
  bool mDownscaleSmooth;
};

typedef rtRef<pxTexture> pxTextureRef;

#endif //PX_TEXTURE_H
