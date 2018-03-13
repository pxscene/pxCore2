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

// pxConfigNative.h

#ifndef PX_CONFIGNATIVE_H
#define PX_CONFIGNATIVE_H

#define PXCALL

//#define PX_LITTLEENDIAN_PIXELS
//#define PX_LITTLEENDIAN_RGBA_PIXELS

#ifndef PX_NATIVE


#if defined(ENABLE_GLUT)

  #include "pxBufferNative.h"
  #include "pxOffscreenNative.h"
  #include "pxWindowNativeGlut.h"

#elif defined(ENABLE_DFB)

#undef RT_DEFAULT_PIX
#define RT_DEFAULT_PIX  RT_PIX_ARGB

  #include "pxBufferNativeDfb.h"
  #include "pxOffscreenNativeDfb.h"

#ifdef ENABLE_DFB_GENERIC
  #include "../generic/LinuxKeyCodes.h"
#else
  #include "pxWindowNativeDfb.h"
#endif //ENABLE_DFB_GENERIC

#else

  #include "pxBufferNative.h"
  #include "pxOffscreenNative.h"
  #include "pxWindowNative.h"

#endif
  #include "pxClipboardNative.h"
#endif // PX_NATIVE

#endif // PX_CONFIGNATIVE_H

