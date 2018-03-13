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

// pxEventLoopNative.cpp

#include "../pxEventLoop.h"

#ifdef PX_PLATFORM_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#endif //PX_PLATFORM_X11

#include "../pxOffscreen.h"

#if defined(ENABLE_GLUT)
  #include "pxWindowNativeGlut.h"
#elif defined(ENABLE_DFB_GENERIC)
  #include "../generic/LinuxKeyCodes.h"
#elif defined(ENABLE_DFB) 
  #include "pxWindowNativeDfb.h"
#else
  #include "pxWindowNative.h"
#endif

void pxEventLoop::run()
{
    // For now we delegate off to the x11 pxWindowNative class
#ifndef ENABLE_DFB_GENERIC
    pxWindowNative::runEventLoop();
#endif //!ENABLE_DFB_GENERIC
}

void pxEventLoop::exit()
{
    // For now we delegate off to the x11 pxWindowNative class
#ifndef ENABLE_DFB_GENERIC
    pxWindowNative::exitEventLoop();
#endif //!ENABLE_DFB_GENERIC
}


///////////////////////////////////////////
// Entry Point 

#ifndef ENABLE_DFB_GENERIC
int main(int argc, char* argv[])
{
  return pxMain(argc, argv);
}
#endif
