// pxCore CopyRight 2005-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxEventLoopNative.cpp

#include "../pxEventLoop.h"
#include "../pxOffscreen.h"
#include "pxWindowNative.h"

void pxEventLoop::run()
{
  // For now we delegate off to the pxWindowNative class
  pxWindowNative::runEventLoop();
}

void pxEventLoop::runOnce()
{
  pxWindowNative::runEventLoopOnce();
}

void pxEventLoop::exit()
{
  // For now we delegate off to the pxWindowNative class
  pxWindowNative::exitEventLoop();
}


///////////////////////////////////////////
// Entry Point 

int main(int argc, char* argv[])
{
  return pxMain(argc, argv);
}
