#ifndef PX_CONTEXT_H
#define PX_CONTEXT_H

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxMatrix4T.h"
#include "rtCore.h"
#include "rtRefT.h"
#include "pxTextureRef.h"

enum pxStretch { PX_NONE = 0, PX_STRETCH = 1, PX_REPEAT = 2 };

class pxContext {
 public:

  pxContext(): mShowOutlines(false) {}

  void init();
  
  bool showOutlines() { return mShowOutlines; }
  void setShowOutlines(bool v) { mShowOutlines = v; }

  void setSize(int w, int h);
  void clear(int w, int h);

  void setMatrix(pxMatrix4f& m);
  void setAlpha(float a);
  
  pxError createContextSurface(pxContextSurfaceNativeDesc* contextSurface, int width, int height);
  pxError setRenderSurface(pxContextSurfaceNativeDesc* contextSurface);
  pxError unsetRenderSurface(pxContextSurfaceNativeDesc* contextSurface);
  pxError deleteContextSurface(pxContextSurfaceNativeDesc* contextSurface);

  rtRefT<pxTextureRef> createTexture(pxOffscreen o);

  void drawRect(float w, float h, float lineWidth, float* fillColor, float* lineColor);
  void drawImage(float w, float h, pxOffscreen& o, 
                 pxStretch xStretch, pxStretch yStretch);
  void drawImage(float w, float h, rtRefT<pxTextureRef> t,
                 pxStretch xStretch, pxStretch yStretch);
  void drawImage9(float w, float h, float x1, float y1,
                  float x2, float y2, pxOffscreen& o);
  void drawSurface(float w, float h, pxContextSurfaceNativeDesc* contextSurface);
  void drawImageAlpha(float x, float y, float w, float h, int bw, int bh, void* buffer, float* color);

// Only use for debug/diag purposes not for normal rendering
  void drawDiagRect(float x, float y, float w, float h, float* color);
  void drawDiagLine(float x1, float y1, float x2, float y2, float* color);

private:
  bool mShowOutlines;

};


#endif
