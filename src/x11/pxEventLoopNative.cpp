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
