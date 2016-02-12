// pxCore Copyright 2007-2015 John Robinson
// pxContext.h

#ifndef PX_CONTEXT_H
#define PX_CONTEXT_H

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxMatrix4T.h"
#include "rtCore.h"
#include "rtRefT.h"
#include "pxConstants.h"
#include "pxTexture.h"
#include "pxContextFramebuffer.h"

//enum pxStretch { PX_NONE = 0, PX_STRETCH = 1, PX_REPEAT = 2 };

class pxContext {
 public:

  pxContext(): mShowOutlines(false) {}

  void init();
  
  // debugging outlines 
  bool showOutlines() { return mShowOutlines; }
  void setShowOutlines(bool v) { mShowOutlines = v; }

  void setSize(int w, int h);
  void clear(int w, int h);
  void clear(int left, int top, int width, int height);

  void setMatrix(pxMatrix4f& m);
  pxMatrix4f getMatrix();
  void setAlpha(float a);
  float getAlpha();

  void pushState();
  void popState();

  pxContextFramebufferRef createFramebuffer(int width, int height);
  pxError updateFramebuffer(pxContextFramebufferRef fbo, int width, int height);
  pxError setFramebuffer(pxContextFramebufferRef fbo);
  pxContextFramebufferRef getCurrentFramebuffer();

  void mapToScreenCoordinates(float inX, float inY, int &outX, int &outY);
  void mapToScreenCoordinates(pxMatrix4f& m, float inX, float inY, int &outX, int &outY);
  bool isObjectOnScreen(float x, float y, float width, float height);

  pxTextureRef createTexture(); // default to use before image load is complete
  pxTextureRef createTexture(pxOffscreen& o);
  pxTextureRef createTexture(float w, float h, float iw, float ih, void* buffer);

  void snapshot(pxOffscreen& o);

  void drawRect(float w, float h, float lineWidth, float* fillColor, float* lineColor);

  void drawImage(float x, float y, float w, float h, pxTextureRef t, pxTextureRef mask, bool useTextureDimsAlways = true,
                 float* color = NULL, pxConstantsStretch::constants xStretch = pxConstantsStretch::NONE, 
                 pxConstantsStretch::constants yStretch = pxConstantsStretch::NONE );

  void drawImage9(float w, float h, float x1, float y1,
                  float x2, float y2, pxTextureRef texture);

// Only use for debug/diag purposes not for normal rendering
  void drawDiagRect(float x, float y, float w, float h, float* color);
  void drawDiagLine(float x1, float y1, float x2, float y2, float* color);

private:
  bool mShowOutlines;
};


#endif
