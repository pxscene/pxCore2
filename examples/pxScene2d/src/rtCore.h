#ifndef RT_CORE_H
#define RT_CORE_H

#include "rtConfig.h"

#if defined(RT_PLATFORM_LINUX)
#include "linux/rtConfigNative.h"
#else
#error "PX_PLATFORM NOT HANDLED"
#endif

#endif
