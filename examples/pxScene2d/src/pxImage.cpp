// pxCore CopyRight 2007-2015 John Robinson
// pxImage.cpp

#include "rtString.h"
#include "rtRefT.h"
#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxScene2d.h"

#include "pxImage.h"

void drawRect(float x, float y, float w, float h, pxOffscreen& o);

void pxImage::url(rtString& s) { s = mURL; }
void pxImage::setURL(const char* s) { 
  mURL = s;
  if (pxLoadImage(s, mOffscreen) != RT_OK)
    printf("image load failed\n");
  else
    printf("image %d, %d\n", mOffscreen.width(), mOffscreen.height());
}

void pxImage::draw() {
  drawRect(0, 0, mOffscreen.width(), mOffscreen.height(), mOffscreen);
}
