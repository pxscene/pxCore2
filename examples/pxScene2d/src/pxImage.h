// pxCore CopyRight 2007-2015 John Robinson
// pxImage.h

#ifndef PX_IMAGE_H
#define PX_IMAGE_H

#include "pxOffscreen.h"
#include "pxContext.h"

class pxImage: public pxObject {
public:
  rtDeclareObject(pxImage, pxObject);
  rtProperty(url, url, setURL, rtString);
  rtProperty(xStretch, xStretch, setXStretch, int32_t);
  rtProperty(yStretch, yStretch, setYStretch, int32_t);
  
pxImage() : mXStretch(PX_NONE), mYStretch(PX_NONE) {}
  
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
  
protected:
  virtual void draw();
  
  rtString mURL;
  pxOffscreen mOffscreen;
  pxStretch mXStretch;
  pxStretch mYStretch;
};

#endif
