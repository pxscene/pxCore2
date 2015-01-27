// pxCore CopyRight 2007-2015 John Robinson
// pxImage.h

#ifndef PX_IMAGE_H
#define PX_IMAGE_H

#include "pxOffscreen.h"

class pxImage: public pxObject {
public:
  rtDeclareObject(pxImage, pxObject);
  rtProperty(url, url, setURL, rtString);

  pxImage() {}
  
  rtError url(rtString& s) const;
  rtError setURL(const char* s);

#if 0
  int width()  { return mOffscreen.width();  }
  int height() { return mOffscreen.height(); } 
#endif

protected:
  virtual void draw();
  
  rtString mURL;
  pxOffscreen mOffscreen;
};

#endif
