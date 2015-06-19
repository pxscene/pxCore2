#ifndef PX_TEXTURE_CACHE_OBJECT_H
#define PX_TEXTURE_CACHE_OBJECT_H

#include "pxTexture.h"
#include "rtError.h"
#include "rtString.h"
#include "rtCore.h"
#include "rtMutex.h"

class pxObject;

class pxFileDownloadRequest;

typedef struct _ImageDownloadRequest
{
  pxFileDownloadRequest* fileDownloadRequest;
  pxTextureRef texture;
} ImageDownloadRequest;

#define RT_TEXTURE_STATUS_OK             0
#define RT_TEXTURE_STATUS_DOWNLOADING    1
#define RT_TEXTURE_STATUS_FILE_NOT_FOUND 2
#define RT_TEXTURE_STATUS_NETWORK_ERROR  3
#define RT_TEXTURE_STATUS_DECODE_FAILURE 4
#define RT_TEXTURE_STATUS_HTTP_ERROR     5
#define RT_TEXTURE_STATUS_UNKNOWN_ERROR  6

class pxTextureCacheObject
{
public:
  pxTextureCacheObject() : mRef(0), mTexture(), mURL(),
        mImageDownloadRequest(NULL), mParent(NULL), mStatusCode(0), mHttpStatusCode(0) {}
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
  int getStatusCode();
  int getHttpStatusCode();


  void onImageDownloadComplete(ImageDownloadRequest imageDownloadRequest);

  static void checkForCompletedDownloads(int maxTimeInMilliseconds=10);
  
protected:
  void loadImage(rtString url);
  void setStatus(int statusCode, int httpStatusCode=0);
  
  rtAtomic mRef;
  pxTextureRef mTexture;
  rtString mURL;
  pxFileDownloadRequest* mImageDownloadRequest;
  pxObject* mParent;
  int mStatusCode;
  int mHttpStatusCode;
};

#endif //PX_TEXTURE_CACHE_OBJECT_H