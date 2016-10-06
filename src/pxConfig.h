// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxConfig.h

#ifndef PX_CONFIG_H
#define PX_CONFIG_H

#if defined(PX_PLATFORM_WIN)
#elif defined(PX_PLATFORM_MAC)
#elif defined(PX_PLATFORM_X11)
#elif defined(PX_PLATFORM_GLUT)
#elif defined(PX_PLATFORM_WAYLAND)
#elif defined(PX_PLATFORM_WAYLAND_EGL)
#elif defined(PX_PLATFORM_GENERIC_EGL)
#elif defined(PX_PLATFORM_GENERIC_DFB)
#elif defined(PX_PLATFORM_DFB_NON_X11)
#else
#error "Must define one of PX_PLATFORM_WIN, PX_PLATFORM_MAC, PX_PLATFORM_X11, PX_PLATFORM_GLUT, PX_PLATFORM_WAYLAND, PX_PLATFORM_WAYLAND_EGL, PX_PLATFORM_GENERIC_DFB PX_PLATFORM_DFB_NON_X11"
#endif

#ifndef PX_PLATFORM_GENERIC_EGL
#define RT_STDCALL //__stdcall
#else
#define RT_STDCALL
#endif

#endif

