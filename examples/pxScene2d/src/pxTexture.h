#ifndef PX_TEXTURE_H
#define PX_TEXTURE_H

#include "pxCore.h"
#include "rtRefT.h"
#include "pxOffscreen.h"

enum pxTextureType { 
  PX_TEXTURE_UNKNOWN = 0,
  PX_TEXTURE_OFFSCREEN = 1, 
  PX_TEXTURE_ALPHA = 2, 
  PX_TEXTURE_NATIVE = 3,
  PX_TEXTURE_FRAME_BUFFER = 4
};

class pxTexture
{
public:
  pxTexture() : mRef(0), mTextureType(PX_TEXTURE_UNKNOWN) {}
  virtual ~pxTexture() {}
  virtual unsigned long AddRef() { return ++mRef; }
  virtual unsigned long Release() { if (--mRef == 0) delete this; return mRef; }
  
  virtual pxError bindTexture() = 0;
  virtual pxError deleteTexture() = 0;
  virtual float width() = 0;
  virtual float height() = 0;
  virtual pxError getOffscreen(pxOffscreen& o) = 0;
  pxTextureType getType() { return mTextureType; }
  virtual pxError prepareForRendering() { return PX_OK; }
  
protected:
  unsigned long mRef;
  pxTextureType mTextureType;
};

typedef rtRefT<pxTexture> pxTextureRef;

#endif //PX_TEXTURE_H