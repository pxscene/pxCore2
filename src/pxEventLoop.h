// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxEventLoop.h

#ifndef PX_EVENTLOOP_H
#define PX_EVENTLOOP_H

// Prototype for the portable entry point for applications using the
// pxCore framework
int pxMain();

// Class used to manage an application's main event loop
class pxEventLoop
{
public:
  void run();
  void exit();

  void runOnce();
};

#endif

