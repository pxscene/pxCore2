/*

 pxCore Copyright 2005-2018 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// pxContext.h

#ifndef PX_CONTEXT_H
#define PX_CONTEXT_H

#include "rtCore.h"
#include "rtRef.h"

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxMatrix4T.h"
#include "pxConstants.h"
#include "pxTexture.h"
#include "pxContextFramebuffer.h"

#ifdef ENABLE_DFB
#include "pxContextDescDFB.h"
#else
#include "pxContextDescGL.h"
#endif //ENABLE_DFB


#define MAX_TEXTURE_WIDTH  2048
#define MAX_TEXTURE_HEIGHT 2048

#define DEFAULT_EJECT_TEXTURE_AGE 5

#ifndef ENABLE_DFB
  #define PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_IN_BYTES (65 * 1024 * 1024)   // GL
  #define PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_THRESHOLD_PADDING_IN_BYTES (5 * 1024 * 1024)
#else
  #define PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_IN_BYTES (15 * 1024 * 1024)   // DFB .. Shoul be 40 ?
  #define PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_THRESHOLD_PADDING_IN_BYTES (5 * 1024 * 1024)
#endif

//enum pxStretch { PX_NONE = 0, PX_STRETCH = 1, PX_REPEAT = 2 };

class pxContext {
 public:

  pxContext()
  : mShowOutlines(false)
  , mCurrentTextureMemorySizeInBytes(0)
  , mTextureMemoryLimitInBytes(PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_IN_BYTES)
  , mTextureMemoryLimitThresholdPaddingInBytes(PXSCENE_DEFAULT_TEXTURE_MEMORY_LIMIT_THRESHOLD_PADDING_IN_BYTES)
#ifdef ENABLE_PX_SCENE_TEXTURE_USAGE_MONITORING
  , mEnableTextureMemoryMonitoring(true)
#else
  , mEnableTextureMemoryMonitoring(false)
#endif
  , mEjectTextureAge(DEFAULT_EJECT_TEXTURE_AGE)
  {}
  ~pxContext();

  void init();
  void term();

  // debugging outlines 
  bool showOutlines() { return mShowOutlines; }
  void setShowOutlines(bool v) { mShowOutlines = v; }

  void setSize(int w, int h);
  void getSize(int& w, int& h);
  void clear(int w, int h);
  void clear(int w, int h, float *fillColor );
  void clear(int left, int top, int width, int height);
  void enableClipping(bool enable);

  void setMatrix(pxMatrix4f& m);
  pxMatrix4f getMatrix();
  void setAlpha(float a);
  float getAlpha();

  void pushState();
  void popState();

  pxContextFramebufferRef createFramebuffer(int width, int height, bool antiAliasing=false, bool alphaOnly=false);
  pxError updateFramebuffer(pxContextFramebufferRef fbo, int width, int height);
  pxError setFramebuffer(pxContextFramebufferRef fbo);
  pxContextFramebufferRef getCurrentFramebuffer();

  void mapToScreenCoordinates(float inX, float inY, int &outX, int &outY);
  void mapToScreenCoordinates(pxMatrix4f& m, float inX, float inY, int &outX, int &outY);
  bool isObjectOnScreen(float x, float y, float width, float height);

  pxTextureRef createTexture(); // default to use before image load is complete
  pxTextureRef createTexture(pxOffscreen& o);
  pxTextureRef createTexture(pxOffscreen& o, const char *compressedData, size_t compressedDataSize);
  pxTextureRef createTexture(float w, float h, float iw, float ih, void* buffer = NULL);

  void snapshot(pxOffscreen& o);

  void drawRect(float w, float h, float lineWidth, float* fillColor, float* lineColor);

  // conveinience method
  void drawImageMasked(float x, float y, float w, float h,
                        pxConstantsMaskOperation::constants maskOp,
                        pxTextureRef t, pxTextureRef mask);
  
  void drawImage(float x, float y, float w, float h, pxTextureRef t,
                 pxTextureRef mask, bool useTextureDimsAlways = true, float* color = NULL,
                 pxConstantsStretch::constants xStretch = pxConstantsStretch::STRETCH,
                 pxConstantsStretch::constants yStretch = pxConstantsStretch::STRETCH,
                 bool downscaleSmooth = false,
                 pxConstantsMaskOperation::constants maskOp= pxConstantsMaskOperation::NORMAL);

#ifdef PXSCENE_FONT_ATLAS
  // This is intended to draw numQuads from the same texture.
  // vertices and uvs for the quads are passed in as if the quads will be rendered
  // using GL_TRIANGLES in an optimal way.  quad oriented backends can skip vertices appropriately
  // 6 vertices (12 floats) and 6 uvs (12 floats) per quad
  void drawTexturedQuads(int numQuads, const void *verts, const void* uvs,
                          pxTextureRef t, float* color);
#endif                          

  void drawImage9(float w, float h, float x1, float y1,
                  float x2, float y2, pxTextureRef texture);

  void drawImage9Border(float w, float h, 
                  float bx1, float by1, float bx2, float by2,
                  float ix1, float iy1, float ix2, float iy2,
                  bool drawCenter, float* color,
                  pxTextureRef texture);

  void drawOffscreen(float src_x, float src_y,
                     float dst_x, float dst_y,
                     float w, float h,
                     pxOffscreen  &offscreen);
  
// Only use for debug/diag purposes not for normal rendering
  void drawDiagRect(float x, float y, float w, float h, float* color);
  void drawDiagLine(float x1, float y1, float x2, float y2, float* color);
  void enableDirtyRectangles(bool enable);
  void adjustCurrentTextureMemorySize(int64_t changeInBytes, bool allowGarbageCollect=true);
  void setTextureMemoryLimit(int64_t textureMemoryLimitInBytes);
  bool isTextureSpaceAvailable(pxTextureRef texture, bool allowGarbageCollect=true, int32_t bytesPerPixel=4);
  int64_t currentTextureMemoryUsageInBytes();
  int64_t textureMemoryOverflow(pxTextureRef texture);
  int64_t ejectTextureMemory(int64_t bytesRequested, bool forceEject=false);
  
  pxError setEjectTextureAge(uint32_t age);
  pxError enableInternalContext(bool enable);

private:
  bool mShowOutlines;
  int64_t mCurrentTextureMemorySizeInBytes;
  int64_t mTextureMemoryLimitInBytes;
  int64_t mTextureMemoryLimitThresholdPaddingInBytes;
  bool mEnableTextureMemoryMonitoring;
  uint32_t mEjectTextureAge;
};


#endif
