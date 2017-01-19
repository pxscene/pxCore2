// rtCore Copyright 2007-2015 John Robinson
// rtCore.h

#ifndef RT_CORE_H
#define RT_CORE_H

#include "rtConfig.h"

#if defined(RT_PLATFORM_LINUX)
#include "unix/rtConfigNative.h"
#elif defined (RT_PLATFORM_WINDOWS)
#include "win/rtConfigNative.h"
#else
#error "PX_PLATFORM NOT HANDLED"
#endif

#endif
