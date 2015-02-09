// pxCore CopyRight 2005-2006 John Robinson
// Portable Framebuffer and Windowing Library
// pxWindowNativeGlut.h

#ifndef PX_WINDOW_NATIVE_GLUT_H
#define PX_WINDOW_NATIVE_GLUT_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>

#include <stdio.h>
#include <sys/mman.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <linux/input.h>
#include <time.h>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glext.h>

using namespace std;



// Since the lifetime of the Display should include the lifetime of all windows
// and eventloop that uses it - refcounting is utilized through this
// wrapper class.
typedef struct _glutDisplay
{
  _glutDisplay() {}
}
glutDisplay;

class displayRef
{
public:
  displayRef();
  ~displayRef();

  glutDisplay* getDisplay() const;

private:

  pxError createGlutDisplay();
  void cleanupGlutDisplay();

  static glutDisplay* mDisplay;
  static int mRefCount;
};

class pxWindowNative
{
public:
  pxWindowNative(): mTimerFPS(0), mLastWidth(-1), mLastHeight(-1),
    mResizeFlag(false), mLastAnimationTime(0.0), mVisible(false),
    mGlutWindowId(0)
  { }

  virtual ~pxWindowNative();

  // Contract between pxEventLoopNative and this class
  static void runEventLoop();
  static void exitEventLoop();

  static struct wl_shell_surface_listener mShellSurfaceListener;

  static vector<pxWindowNative*> getNativeWindows(){return mWindowVector;}

  virtual void onMouseDown(int x, int y, unsigned long flags) = 0;
  virtual void onMouseUp(int x, int y, unsigned long flags) = 0;

  virtual void onMouseMove(int x, int y) = 0;

  virtual void onMouseLeave() = 0;

  virtual void onKeyDown(int keycode, unsigned long flags) = 0;
  virtual void onKeyUp(int keycode, unsigned long flags) = 0;

  virtual void onSize(int w, int h) = 0;

  void animateAndRender();

protected:
  virtual void onCreate() = 0;

  virtual void onCloseRequest() = 0;
  virtual void onClose() = 0;

  virtual void onDraw(pxSurfaceNative surface) = 0;

  virtual void onAnimationTimer() = 0;

  void onAnimationTimerInternal();

  void invalidateRectInternal(pxRect *r);
  double getLastAnimationTime();
  void setLastAnimationTime(double time);
  void drawFrame();


  displayRef mDisplayRef;

  int mTimerFPS;
  int mLastWidth, mLastHeight;
  bool mResizeFlag;
  double mLastAnimationTime;
  bool mVisible;

  //timer variables
  static bool mEventLoopTimerStarted;
  static float mEventLoopInterval;
  static timer_t mRenderTimerId;

  void createGlutWindow(int left, int top, int width, int height);
  void cleanupGlutWindow();

  int mGlutWindowId;

  static void registerWindow(pxWindowNative* p);
  static void unregisterWindow(pxWindowNative* p); //call this method somewhere
  static vector<pxWindowNative*> mWindowVector;
};

// Key Codes
#define PX_KEY_NATIVE_ENTER        13
#define PX_KEY_NATIVE_BACKSPACE    8
#define PX_KEY_NATIVE_TAB          9
#define PX_KEY_NATIVE_CANCEL       10001 //TODO
#define PX_KEY_NATIVE_CLEAR        10002 //TODO
#define PX_KEY_NATIVE_SHIFT        10030 //TODO - special
#define PX_KEY_NATIVE_CONTROL      10031 //TODO - special
#define PX_KEY_NATIVE_ALT          10032 //TODO - special
#define PX_KEY_NATIVE_PAUSE        19
#define PX_KEY_NATIVE_CAPSLOCK     20
#define PX_KEY_NATIVE_ESCAPE       27
#define PX_KEY_NATIVE_SPACE        32
#define PX_KEY_NATIVE_PAGEUP       33
#define PX_KEY_NATIVE_PAGEDOWN     34
#define PX_KEY_NATIVE_END          35
#define PX_KEY_NATIVE_HOME         36
#define PX_KEY_NATIVE_LEFT         10020 //TODO - special
#define PX_KEY_NATIVE_UP           10021 //TODO - special
#define PX_KEY_NATIVE_RIGHT        10022 //TODO - special
#define PX_KEY_NATIVE_DOWN         10023 //TODO - special
#define PX_KEY_NATIVE_COMMA        44
#define PX_KEY_NATIVE_PERIOD       46
#define PX_KEY_NATIVE_SLASH        47
#define PX_KEY_NATIVE_ZERO         48
#define PX_KEY_NATIVE_ONE          49
#define PX_KEY_NATIVE_TWO          50
#define PX_KEY_NATIVE_THREE        51
#define PX_KEY_NATIVE_FOUR         52
#define PX_KEY_NATIVE_FIVE         53
#define PX_KEY_NATIVE_SIX          54
#define PX_KEY_NATIVE_SEVEN        55
#define PX_KEY_NATIVE_EIGHT        56
#define PX_KEY_NATIVE_NINE         57
#define PX_KEY_NATIVE_SEMICOLON    59
#define PX_KEY_NATIVE_EQUALS       61
#define PX_KEY_NATIVE_A            65
#define PX_KEY_NATIVE_B            66
#define PX_KEY_NATIVE_C            67
#define PX_KEY_NATIVE_D            68
#define PX_KEY_NATIVE_E            69
#define PX_KEY_NATIVE_F            70
#define PX_KEY_NATIVE_G            71
#define PX_KEY_NATIVE_H            72
#define PX_KEY_NATIVE_I            73
#define PX_KEY_NATIVE_J            74
#define PX_KEY_NATIVE_K            75
#define PX_KEY_NATIVE_L            76
#define PX_KEY_NATIVE_M            77
#define PX_KEY_NATIVE_N            78
#define PX_KEY_NATIVE_O            79
#define PX_KEY_NATIVE_P            80
#define PX_KEY_NATIVE_Q            81
#define PX_KEY_NATIVE_R            82
#define PX_KEY_NATIVE_S            83
#define PX_KEY_NATIVE_T            84
#define PX_KEY_NATIVE_U            85
#define PX_KEY_NATIVE_V            86
#define PX_KEY_NATIVE_W            87
#define PX_KEY_NATIVE_X            88
#define PX_KEY_NATIVE_Y            89
#define PX_KEY_NATIVE_Z            90
#define PX_KEY_NATIVE_OPENBRACKET  91
#define PX_KEY_NATIVE_BACKSLASH    92
#define PX_KEY_NATIVE_CLOSEBRACKET 93
#define PX_KEY_NATIVE_NUMPAD0      10003 //TODO
#define PX_KEY_NATIVE_NUMPAD1      10004 //TODO
#define PX_KEY_NATIVE_NUMPAD2      10005 //TODO
#define PX_KEY_NATIVE_NUMPAD3      10006 //TODO
#define PX_KEY_NATIVE_NUMPAD4      10007 //TODO
#define PX_KEY_NATIVE_NUMPAD5      10008 //TODO
#define PX_KEY_NATIVE_NUMPAD6      10009 //TODO
#define PX_KEY_NATIVE_NUMPAD7      10010 //TODO
#define PX_KEY_NATIVE_NUMPAD8      10011 //TODO
#define PX_KEY_NATIVE_NUMPAD9      10012 //TODO
#define PX_KEY_NATIVE_MULTIPLY     10013 //TODO
#define PX_KEY_NATIVE_ADD          10014 //TODO
#define PX_KEY_NATIVE_SEPARATOR    45 //XK_KP_Separator
#define PX_KEY_NATIVE_SUBTRACT     10015 //TODO
#define PX_KEY_NATIVE_DECIMAL      10016 //TODO
#define PX_KEY_NATIVE_DIVIDE       10017 //TODO
#define PX_KEY_NATIVE_F1           0x0001
#define PX_KEY_NATIVE_F2           0x0002
#define PX_KEY_NATIVE_F3           0x0003
#define PX_KEY_NATIVE_F4           0x0004
#define PX_KEY_NATIVE_F5           0x0005
#define PX_KEY_NATIVE_F6           0x0006
#define PX_KEY_NATIVE_F7           0x0007
#define PX_KEY_NATIVE_F8           10040 //TODO - special
#define PX_KEY_NATIVE_F9           10041 //TODO - special
#define PX_KEY_NATIVE_F10          0x000A
#define PX_KEY_NATIVE_F11          0x000B
#define PX_KEY_NATIVE_F12          0x000C
#define PX_KEY_NATIVE_DELETE       10050 //TODO - special
#define PX_KEY_NATIVE_NUMLOCK      10051 //TODO - special
#define PX_KEY_NATIVE_SCROLLLOCK   10018 //TODO
#define PX_KEY_NATIVE_PRINTSCREEN  0xfd1d
#define PX_KEY_NATIVE_INSERT       10052 //TODO - special
#define PX_KEY_NATIVE_HELP         10019 //TODO
#define PX_KEY_NATIVE_BACKQUOTE    96
#define PX_KEY_NATIVE_QUOTE        39

#endif // PX_WINDOW_NATIVE_GLUT_H
