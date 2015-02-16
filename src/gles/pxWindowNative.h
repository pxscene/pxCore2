#ifndef PX_WINDOW_NATIVE_H
#define PX_WINDOW_NATIVE_H

#include "pxBufferNative.h"
#include "pxEGLProvider.h"
#include "pxInputDeviceEventProvider.h"

#include <vector>

using namespace std;

class pxWindowNative
{
public:
  pxWindowNative();
  virtual ~pxWindowNative();

  static void runEventLoop();
  static void exitEventLoop();

  //timer methods
  static int createAndStartEventLoopTimer(int timeoutInMilliseconds);
  static int stopAndDeleteEventLoopTimer();

  void animateAndRender();
  
  static vector<pxWindowNative*> getNativeWindows(){return mWindowVector;}

protected:
  virtual void onCreate() = 0;
  virtual void onCloseRequest() = 0;
  virtual void onClose() = 0;
  virtual void onAnimationTimer() = 0;	
  virtual void onSize(int w, int h) = 0;

  virtual void onMouseDown(int x, int y, unsigned long flags) = 0;
  virtual void onMouseUp(int x, int y, unsigned long flags) = 0;
  virtual void onMouseLeave() = 0;
  virtual void onMouseMove(int x, int y) = 0;

  virtual void onKeyDown(int keycode, unsigned long flags) = 0;
  virtual void onKeyUp(int keycode, unsigned long flags) = 0;
  virtual void onChar(char c) = 0;
  virtual void onDraw(pxSurfaceNative surface) = 0;

  void onAnimationTimerInternal();
  void invalidateRectInternal(pxRect *r);
  double getLastAnimationTime();
  void setLastAnimationTime(double time);
  void drawFrame();
  
  static void registerWindow(pxWindowNative* p);
  static void unregisterWindow(pxWindowNative* p); //call this method somewhere
  static vector<pxWindowNative*> mWindowVector;
  
  int mTimerFPS;
  int mLastWidth, mLastHeight;
  bool mResizeFlag;
  double mLastAnimationTime;
  bool mVisible;

private:
  static void keyEventListener(const pxKeyEvent& evt, void* argp);
  static void mouseEventListener(const pxMouseEvent& evt, void* argp);
  static void* dispatchInput(void* argp);
  void dispatchInputEvents();

private:
  // generic egl stuff
  EGLDisplay mEGLDisplay;
  EGLSurface mEGLSurface;
  EGLContext mEGLContext;

  pxInputDeviceEventProvider* mInputProvider;

  // TODO: rtThread;
  pthread_t mInputEventThread;
    
  //timer variables
  static bool mEventLoopTimerStarted;
  static float mEventLoopInterval;
  static timer_t mRenderTimerId;

protected:
  pxEGLProvider* mEGLProvider;

};

#include "LinuxKeyCodes.h"

#endif
