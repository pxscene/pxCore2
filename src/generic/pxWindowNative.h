#ifndef PX_WINDOW_NATIVE_H
#define PX_WINDOW_NATIVE_H

#include "pxBufferNative.h"

#include <vector>

using namespace std;

class pxWindowNative
{
public:
  pxWindowNative();
  virtual ~pxWindowNative();

  static void runEventLoopOnce();
  static void runEventLoop();
  static void exitEventLoop();

  //timer methods
  static int createAndStartEventLoopTimer(int timeoutInMilliseconds);
  static int stopAndDeleteEventLoopTimer();

  void animateAndRender();
  
protected:
  virtual void onCreate() = 0;
  virtual void onCloseRequest() = 0;
  virtual void onClose() = 0;
  virtual void onAnimationTimer() = 0;	
  virtual void onSize(int32_t w, int32_t h) = 0;

  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags) = 0;
  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags) = 0;
  virtual void onMouseLeave() = 0;
  virtual void onMouseMove(int32_t x, int32_t y) = 0;

  virtual void onKeyDown(uint32_t keycode, uint32_t flags) = 0;
  virtual void onKeyUp(uint32_t keycode, uint32_t flags) = 0;
  virtual void onChar(uint32_t c) = 0;
  virtual void onDraw(pxSurfaceNative surface) = 0;

  void onAnimationTimerInternal();
  void invalidateRectInternal(pxRect *r);
  double getLastAnimationTime();
  void setLastAnimationTime(double time);
  void drawFrame();

private:
  static void* dispatchInput(void* argp);
  void dispatchInputEvents();
};


#endif
