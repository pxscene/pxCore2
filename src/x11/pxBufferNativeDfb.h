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

// pxBufferNativeDfb.h

#ifndef PX_BUFFER_NATIVE_DFB_H
#define PX_BUFFER_NATIVE_DFB_H

#include <directfb.h>

#include "../pxCore.h"

// Structure used to describe a window surface under X11
typedef struct
{
  IDirectFB          *dfb;
  IDirectFBSurface   *surface;

  int windowWidth;
  int windowHeight;

} pxSurfaceNativeDesc;

typedef pxSurfaceNativeDesc* pxSurfaceNative;

#endif //PX_BUFFER_NATIVE_DFB_H
