// pxCore CopyRight 2007-2009 John Robinson
// Portable Framebuffer and Windowing Library
// pxConfig.h

#ifndef PX_CONFIG_H
#define PX_CONFIG_H

#if defined(PX_PLATFORM_WIN)
#elif defined(PX_PLATFORM_MAC)
#elif defined(PX_PLATFORM_X11)
#else
#error "Must define one of PX_PLATFORM_WIN, PX_PLATFORM_MAC, PX_PLATFORM_X11"
#endif

#endif

