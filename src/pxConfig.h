/*

 pxCore Copyright 2005-2017 John Robinson

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

// pxConfig.h

#ifndef PX_CONFIG_H
#define PX_CONFIG_H


#define RT_DEFAULT_PIX  RT_PIX_RGBA

#if defined(PX_PLATFORM_WIN)
#elif defined(PX_PLATFORM_MAC)
#elif defined(PX_PLATFORM_X11)
#elif defined(PX_PLATFORM_GLUT)
#elif defined(PX_PLATFORM_ESSOS)
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

