// pxConfigNative.h

#ifndef PX_CONFIGNATIVE_H
#define PX_CONFIGNATIVE_H

#define PXCALL

#define PX_LITTLEENDIAN_PIXELS
#define PX_LITTLEENDIAN_RGBA_PIXELS

#ifdef __APPLE__
#define PX_USE_GLUT_ON_CLOSE
#else
#define PX_USE_SELECT_FOR_SLEEP
#define PX_USE_CGT
#define PX_USE_GLEW
#endif


#ifndef PX_NATIVE

#include "pxBufferNative.h"
#include "pxOffscreenNative.h"
#include "pxWindowNative.h"
#include "pxClipboardNative.h"

#endif // PX_NATIVE

#endif // PX_CONFIGNATIVE_H

