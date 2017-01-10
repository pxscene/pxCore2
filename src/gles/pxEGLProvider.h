#ifndef PX_EGL_PROVIDER_H
#define PX_EGL_PROVIDER_H

#include <EGL/egl.h>
#include "../pxCore.h"

struct pxEGLProvider
{
  virtual ~pxEGLProvider() { }
  virtual pxError initWithDefaults(int width, int height) = 0;
  virtual pxError init() = 0;
  virtual pxError createDisplay(EGLDisplay* display) = 0;
  virtual pxError createSurface(EGLDisplay display, const EGLConfig& config, EGLSurface* surface) = 0;
  virtual pxError createContext(EGLDisplay display, const EGLConfig& config, EGLContext* context) = 0;
};

#endif
