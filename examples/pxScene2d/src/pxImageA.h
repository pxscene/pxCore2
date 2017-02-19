// pxCore Copyright 2007-2015 John Robinson
// pxImageA.h

#ifndef PX_IMAGEA_H
#define PX_IMAGEA_H

#include "pxScene2d.h"

#include "pxFileDownloader.h"
#include "pxUtil.h"


class pxImageA: public pxObject
{
public:
  rtDeclareObject(pxImageA, pxObject);
  rtProperty(url, url, setUrl, rtString);
  rtProperty(stretchX, stretchX, setStretchX, int32_t);
  rtProperty(stretchY, stretchY, setStretchY, int32_t);
  // TODO resource (for animated images)
  //rtProperty(resource, resource, setResource, rtObjectRef);

  pxImageA(pxScene2d* scene);

  virtual void onInit() 
  {
    mw = mImageWidth;
    mh = mImageHeight;
  }

  void sendPromise() {} // shortcircuit  TODO...not sure if I like this pattern
  
  rtError url(rtString& s) const;
  rtError setUrl(const char* s);

  rtError stretchX(int32_t& v) const { v = (int32_t)mStretchX; return RT_OK; }
  rtError setStretchX(int32_t v);

  rtError stretchY(int32_t& v) const { v = (int32_t)mStretchY; return RT_OK; }
  rtError setStretchY(int32_t v);

  virtual void update(double t);
  virtual void draw();
  
private:
  static void onDownloadComplete(pxFileDownloadRequest* downloadRequest);
  static void onDownloadCompleteUI(void* context, void* data);
  
  pxTimedOffscreenSequence mImageSequence;
  uint32_t mCurFrame;
  uint32_t mCachedFrame;
  uint32_t mPlays;

  uint32_t mImageWidth;
  uint32_t mImageHeight;

  pxTextureRef mTexture;

  double mFrameTime;
  rtString mURL;
  pxConstantsStretch::constants mStretchX;
  pxConstantsStretch::constants mStretchY;

  pxFileDownloadRequest* mDownloadRequest;
};

#endif
