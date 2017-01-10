#include "pxCore.h"
#include "pxWindowNative.h"
#include "pxCore.h"
#include "pxWindow.h"
#include "pxWindowUtil.h"
#include "pxTimer.h"

#include <dlfcn.h>
#include <errno.h>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

// TODO figure out what to do with rtLog
#if 0
#include "rtLog.h"
#else
#define rtLogWarn printf
#define rtLogError printf
#define rtLogFatal printf
#define rtLogInfo printf
#endif

#define EGL_PX_CORE_FPS 30

pxWindowNative::pxWindowNative()
{
  
}

pxWindowNative::~pxWindowNative()
{
  
}

pxError pxWindow::init(int /*left*/, int /*top*/, int width, int height)
{
  
  return PX_OK;
}

pxError pxWindow::term()
{
  return PX_OK;
}

void pxWindow::invalidateRect(pxRect *r)
{
  invalidateRectInternal(r);
}

// This can be improved by collecting the dirty regions and painting
// when the event loop goes idle
void pxWindowNative::invalidateRectInternal(pxRect *r)
{
  (void)r;
  //rendering for egl is now handled inside of onWindowTimerFired()
  drawFrame();
}

bool pxWindow::visibility()
{
  return true;
}

void pxWindow::setVisibility(bool visible)
{
}

pxError pxWindow::setAnimationFPS(uint32_t fps)
{
  return PX_OK;
}

void pxWindow::setTitle(const char* /*title*/)
{
  //todo
}

pxError pxWindow::beginNativeDrawing(pxSurfaceNative& s)
{
  (void)s;
  //todo

  return PX_OK;
}

pxError pxWindow::endNativeDrawing(pxSurfaceNative& s)
{
  (void)s;
  //todo

  return PX_OK;
}

// pxWindowNative

void pxWindowNative::onAnimationTimerInternal()
{
}

int pxWindowNative::createAndStartEventLoopTimer(int timeoutInMilliseconds )
{
  

  return(0);
}

int pxWindowNative::stopAndDeleteEventLoopTimer()
{
  int returnValue = 0;
  
  return returnValue;
}

void pxWindowNative::runEventLoopOnce()
{
  usleep(1000); //TODO - find out why pxSleepMS causes a crash on xi3
}

void pxWindowNative::runEventLoop()
{
  
}

void pxWindowNative::exitEventLoop()
{
}

void pxWindowNative::animateAndRender()
{
  //
}

void pxWindowNative::drawFrame()
{
  
}

void pxWindowNative::setLastAnimationTime(double time)
{
}

double pxWindowNative::getLastAnimationTime()
{
  return 0;
}


void* pxWindowNative::dispatchInput(void* argp)
{
  return 0;
}

void pxWindowNative::dispatchInputEvents()
{
}


