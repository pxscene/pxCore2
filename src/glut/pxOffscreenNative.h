// pxCore CopyRight 2005-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxOffscreenNative.h

#ifndef PX_OFFSCREEN_NATIVE_H
#define PX_OFFSCREEN_NATIVE_H

#include "../pxCore.h"
#include "../pxBuffer.h"

class pxOffscreenNative: public pxBuffer
{
public:
pxOffscreenNative(): data(NULL) {}
  virtual ~pxOffscreenNative() {}

  pxError term();

protected:
  char* data;
};

#endif
