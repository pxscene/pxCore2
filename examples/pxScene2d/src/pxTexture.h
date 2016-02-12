#ifndef PX_TEXTURE_H
#define PX_TEXTURE_H

#include "pxCore.h"
#include "rtRefT.h"
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


class pxTexture: public pxTextureNative
{
public:
  pxTexture() : mRef(0), mTextureType(PX_TEXTURE_UNKNOWN), mPremultipliedAlpha(false)
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
  
  virtual pxError bindTexture() { return PX_FAIL; }
  virtual pxError bindTextureAsMask() { return PX_FAIL; }
  virtual pxError deleteTexture() = 0;
  virtual int width() = 0;
  virtual int height() = 0;
  virtual pxError resizeTexture(int w, int h) { (void)w; (void)h; return PX_FAIL; }
  virtual pxError getOffscreen(pxOffscreen& o) = 0;
  virtual unsigned int getNativeId() { return 0; }
  pxTextureType getType() { return mTextureType; }
  virtual pxError prepareForRendering() { return PX_OK; }
  bool premultipliedAlpha() { return mPremultipliedAlpha; }
  void enablePremultipliedAlpha(bool enable) { mPremultipliedAlpha = enable; }
protected:
  rtAtomic mRef;
  pxTextureType mTextureType;
  bool mPremultipliedAlpha;
};

typedef rtRefT<pxTexture> pxTextureRef;

#endif //PX_TEXTURE_H
