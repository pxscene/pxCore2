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
