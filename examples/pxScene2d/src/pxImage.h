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

protected:
  virtual void draw();
  
  rtString mURL;
  pxOffscreen mOffscreen;
};

#endif
