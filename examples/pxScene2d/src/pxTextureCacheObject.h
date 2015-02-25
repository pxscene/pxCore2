#ifndef PX_TEXTURE_CACHE_OBJECT_H
#define PX_TEXTURE_CACHE_OBJECT_H

#include "pxTexture.h"
#include "rtError.h"
#include "rtString.h"
#include "rtCore.h"
#include "rtMutex.h"

class pxObject;

class pxFileDownloadRequest;

class pxTextureCacheObject
{
public:
  pxTextureCacheObject() : mRef(0), mTexture(), mURL(),
        mImageDownloadRequest(NULL), mParent(NULL) {}
  virtual ~pxTextureCacheObject();

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
  
  pxTextureRef getTexture() { return mTexture; }
  void setParent(pxObject* parent);
  
  rtError url(rtString& s) const;
  rtError setURL(const char* s);

  void onFileDownloadComplete(pxFileDownloadRequest* downloadRequest);

  static void checkForCompletedDownloads();
  
protected:
  void loadImage(rtString url);
  
  rtAtomic mRef;
  pxTextureRef mTexture;
  rtString mURL;
  pxFileDownloadRequest* mImageDownloadRequest;
  pxObject* mParent;
};

#endif //PX_TEXTURE_CACHE_OBJECT_H