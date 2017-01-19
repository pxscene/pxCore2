// rtCore Copyright 2007-2015 John Robinson
// rtConfig.h

#ifndef RT_CONFIG_H
#define RT_CONFIG_H

#if defined(RT_PLATFORM_LINUX)
#elif defined(RT_PLATFORM_WINDOWS)
#else
#error "Must define be RT_PLATFORM_LINUX"
#endif

#endif
