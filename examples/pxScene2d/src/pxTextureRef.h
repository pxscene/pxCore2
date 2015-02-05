#ifndef PX_TEXTURE_REF_H
#define PX_TEXTURE_REF_H

#include "pxCore.h"

enum pxTextureRefType { 
  PX_TEXTURE_UNKNOWN = 0,
  PX_TEXTURE_OFFSCREEN = 1, 
  PX_TEXTURE_ALPHA = 2, 
  PX_TEXTURE_NATIVE = 3,
  PX_TEXTURE_FRAME_BUFFER = 4
};

class pxTextureRef
{
public:
  pxTextureRef() : mRef(0), mTextureType(PX_TEXTURE_UNKNOWN) {}
  virtual ~pxTextureRef() {}
  virtual unsigned long AddRef() { return ++mRef; }
  virtual unsigned long Release() { if (--mRef == 0) delete this; return mRef; }
  
  virtual pxError bindTexture() = 0;
  virtual pxError deleteTexture() = 0;
  virtual float getWidth() = 0;
  virtual float getHeight() = 0;
  pxTextureRefType getType() { return mTextureType; }
  
protected:
  unsigned long mRef;
  pxTextureRefType mTextureType;
};

#endif //PX_TEXTURE_REF_H