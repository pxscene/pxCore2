// pxCore CopyRight 2007-2015 John Robinson
// pxImage.h

#ifndef PX_IMAGE_H
#define PX_IMAGE_H

#include "pxOffscreen.h"

class pxImage: public pxObject {
public:
  pxImage() {}
  
  void url(rtString& s);
  void setURL(const char* s);

  int width()  { return mOffscreen.width();  }
  int height() { return mOffscreen.height(); } 

private:
  virtual void draw();
  
  rtString mURL;
  pxOffscreen mOffscreen;
};

#endif
