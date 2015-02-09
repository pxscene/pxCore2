#ifndef PX_EGL_PROVIDER_H
#define PX_EGL_PROVIDER_H

#include <EGL/egl.h>
#include "rtError.h"

struct pxEGLProvider
{
  virtual rtError init() = 0;
  virtual rtError createDisplay(EGLDisplay* display) = 0;
  virtual rtError createSurface(EGLDisplay display, const EGLConfig& config, EGLSurface* surface) = 0;
  virtual rtError createContext(EGLDisplay display, const EGLConfig& config, EGLContext* context) = 0;
};

#endif
