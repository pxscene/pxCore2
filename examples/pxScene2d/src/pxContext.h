#ifndef PX_CONTEXT_H
#define PX_CONTEXT_H

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxMatrix4T.h"
#include "rtCore.h"

enum pxStretch { PX_NONE = 0, PX_STRETCH = 1, PX_REPEAT = 2 };

class pxContext {
 public:
  void init();
  void setSize(int w, int h);
  void clear(int w, int h);

  void setMatrix(pxMatrix4f& m);
  void setAlpha(float a);
  
  pxError createContextSurface(pxContextSurfaceNativeDesc* contextSurface, int width, int height);
  pxError setRenderSurface(pxContextSurfaceNativeDesc* contextSurface);
  pxError unsetRenderSurface(pxContextSurfaceNativeDesc* contextSurface);
  pxError deleteContextSurface(pxContextSurfaceNativeDesc* contextSurface);

  void drawRect(float w, float h, float lineWidth, float* fillColor, float* lineColor);
  void drawImage(float w, float h, pxOffscreen& o, 
                 pxStretch xStretch, pxStretch yStretch);
  void drawImage9(float w, float h, pxOffscreen& o);
  void drawSurface(float w, float h, pxContextSurfaceNativeDesc* contextSurface);
  void drawImageAlpha(float x, float y, float w, float h, int bw, int bh, void* buffer, float* color);
};

#endif
