#ifndef RT_CORE_H
#define RT_CORE_H

#include "rtConfig.h"

#ifdef ENABLE_GRAPHICS_OPENGL
#include "pxSnapshotOpenGL.h"
#else
#include "pxSnapshotNoGraphics.h"
#endif

#if defined(RT_PLATFORM_LINUX)
#include "linux/rtConfigNative.h"
#else
#error "PX_PLATFORM NOT HANDLED"
#endif

#endif
