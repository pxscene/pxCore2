// pxEventLoopNative.cpp

#include "../pxEventLoop.h"

#include "../pxOffscreen.h"
#include "pxWindowNative.h"

void pxEventLoop::runOnce()
{
  pxWindowNative::runEventLoopOnce();
}

void pxEventLoop::run()
{
  // For now we delegate off to the x11 pxWindowNative class
  pxWindowNative::runEventLoop();
}

void pxEventLoop::exit()
{
  // For now we delegate off to the x11 pxWindowNative class
  pxWindowNative::exitEventLoop();
}


///////////////////////////////////////////
// Entry Point 

int main(int argc, char** argv)
{
    return pxMain(argc, argv);
}
