#include "pxTexture.h"
#include "pxTextureCacheObject.h"

extern void removeFromTextureCache(pxTexture* texture);

pxTexture::~pxTexture()
{
  removeFromTextureCache(this);
}