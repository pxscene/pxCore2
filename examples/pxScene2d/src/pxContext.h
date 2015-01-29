#ifndef PX_CONTEXT_H
#define PX_CONTEXT_H

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxMatrix4T.h"

class pxContext {
 public:
  void init();
  void setSize(int w, int h);
  void clear(int w, int h);

  void setMatrix(pxMatrix4f& m);
  void setAlpha(float a);

  void drawRect(float w, float h, float lineWidth, float* fillColor, float* lineColor);
  void drawImage(float w, float h, pxOffscreen& o);
  void drawImage9(float w, float h, pxOffscreen& o);
  void drawImageAlpha(float x, float y, float w, float h, int bw, int bh, void* buffer, float* color);
};

#endif
