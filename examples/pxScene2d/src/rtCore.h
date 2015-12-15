// rtCore Copyright 2007-2015 John Robinson
// rtCore.h

#ifndef RT_CORE_H
#define RT_CORE_H

#include "rtConfig.h"

//todo - add support for DFB
#ifdef ENABLE_DFB
#include "pxContextDescDFB.h"
#else
#include "pxContextDescGL.h"
#endif //ENABLE_DFB

#if defined(RT_PLATFORM_LINUX)
#include "linux/rtConfigNative.h"
#elif defined (RT_PLATFORM_WINDOWS)
#include "win/rtConfigNative.h"
#else
#error "PX_PLATFORM NOT HANDLED"
#endif

#endif
