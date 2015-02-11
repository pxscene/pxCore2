// pxCore CopyRight 2007-2015 John Robinson
// pxImage.h

#ifndef PX_IMAGE_H
#define PX_IMAGE_H

#include "pxContext.h"
#include "rtMutex.h"
#include "pxTexture.h"

class pxImageDownloadRequest;

class pxImage: public pxObject {
public:
  rtDeclareObject(pxImage, pxObject);
  rtProperty(url, url, setURL, rtString);
  rtProperty(xStretch, xStretch, setXStretch, int32_t);
  rtProperty(yStretch, yStretch, setYStretch, int32_t);
  
pxImage() : mXStretch(PX_NONE), mYStretch(PX_NONE), mTexture(),
        mWaitingForImageDownload(false),
        mImageDownloadMutex(), mImageDownloadIsAvailable(false),
        mImageDownloadRequest(NULL) {}

  virtual ~pxImage() { rtLogInfo("~pxImage()"); }
  
  rtError url(rtString& s) const;
  rtError setURL(const char* s);
  
  rtError xStretch(int32_t& v) const { v = (int32_t)mXStretch; return RT_OK; }
  rtError setXStretch(int32_t v)
  {
    mXStretch = (pxStretch)v;
    return RT_OK;
  }

  rtError yStretch(int32_t& v) const { v = (int32_t)mYStretch; return RT_OK; }
  rtError setYStretch(int32_t v)
  {
    mYStretch = (pxStretch)v;
    return RT_OK;
  }
  
  pxTextureRef getTexture()
  {
    return mTexture;
  }
  
  void onImageDownloadComplete(pxImageDownloadRequest* imageDownloadRequest);
  
protected:
  virtual void draw();
  void checkForCompletedImageDownload();
  void loadImage(rtString url);
  
  rtString mURL;
  pxStretch mXStretch;
  pxStretch mYStretch;
  pxTextureRef mTexture;
  
  bool mWaitingForImageDownload;
  rtMutex mImageDownloadMutex;
  bool mImageDownloadIsAvailable;
  pxImageDownloadRequest* mImageDownloadRequest;
};

#endif
