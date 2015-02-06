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
#define PX_KEY_NATIVE_ENTER        XK_Return
#define PX_KEY_NATIVE_BACKSPACE    XK_BackSpace
#define PX_KEY_NATIVE_TAB          XK_Tab
#define PX_KEY_NATIVE_CANCEL       XK_Cancel
#define PX_KEY_NATIVE_CLEAR        XK_Clear
#define PX_KEY_NATIVE_SHIFT        XK_Shift_L
#define PX_KEY_NATIVE_CONTROL      XK_Control_L
#define PX_KEY_NATIVE_ALT          XK_Alt_L
#define PX_KEY_NATIVE_PAUSE        XK_Pause
#define PX_KEY_NATIVE_CAPSLOCK     XK_Caps_Lock
#define PX_KEY_NATIVE_ESCAPE       XK_Escape
#define PX_KEY_NATIVE_SPACE        XK_space
#define PX_KEY_NATIVE_PAGEUP       XK_Page_Up
#define PX_KEY_NATIVE_PAGEDOWN     XK_Page_Down
#define PX_KEY_NATIVE_END          XK_End
#define PX_KEY_NATIVE_HOME         XK_Home
#define PX_KEY_NATIVE_LEFT         XK_Left
#define PX_KEY_NATIVE_UP           XK_Up
#define PX_KEY_NATIVE_RIGHT        XK_Right
#define PX_KEY_NATIVE_DOWN         XK_Down
#define PX_KEY_NATIVE_COMMA        XK_comma
#define PX_KEY_NATIVE_PERIOD       XK_period
#define PX_KEY_NATIVE_SLASH        XK_slash
#define PX_KEY_NATIVE_ZERO         XK_0
#define PX_KEY_NATIVE_ONE          XK_1
#define PX_KEY_NATIVE_TWO          XK_2
#define PX_KEY_NATIVE_THREE        XK_3
#define PX_KEY_NATIVE_FOUR         XK_4
#define PX_KEY_NATIVE_FIVE         XK_5
#define PX_KEY_NATIVE_SIX          XK_6
#define PX_KEY_NATIVE_SEVEN        XK_7
#define PX_KEY_NATIVE_EIGHT        XK_8
#define PX_KEY_NATIVE_NINE         XK_9
#define PX_KEY_NATIVE_SEMICOLON    XK_semicolon
#define PX_KEY_NATIVE_EQUALS       XK_equal
#define PX_KEY_NATIVE_A            XK_A
#define PX_KEY_NATIVE_B            XK_B
#define PX_KEY_NATIVE_C            XK_C
#define PX_KEY_NATIVE_D            XK_D
#define PX_KEY_NATIVE_E            XK_E
#define PX_KEY_NATIVE_F            XK_F
#define PX_KEY_NATIVE_G            XK_G
#define PX_KEY_NATIVE_H            XK_H
#define PX_KEY_NATIVE_I            XK_I
#define PX_KEY_NATIVE_J            XK_J
#define PX_KEY_NATIVE_K            XK_K
#define PX_KEY_NATIVE_L            XK_L
#define PX_KEY_NATIVE_M            XK_M
#define PX_KEY_NATIVE_N            XK_N
#define PX_KEY_NATIVE_O            XK_O
#define PX_KEY_NATIVE_P            XK_P
#define PX_KEY_NATIVE_Q            XK_Q
#define PX_KEY_NATIVE_R            XK_R
#define PX_KEY_NATIVE_S            XK_S
#define PX_KEY_NATIVE_T            XK_T
#define PX_KEY_NATIVE_U            XK_U
#define PX_KEY_NATIVE_V            XK_V
#define PX_KEY_NATIVE_W            XK_W
#define PX_KEY_NATIVE_X            XK_X
#define PX_KEY_NATIVE_Y            XK_Y
#define PX_KEY_NATIVE_Z            XK_Z
#define PX_KEY_NATIVE_OPENBRACKET  XK_bracketleft
#define PX_KEY_NATIVE_BACKSLASH    XK_backslash
#define PX_KEY_NATIVE_CLOSEBRACKET XK_bracketright
#define PX_KEY_NATIVE_NUMPAD0      XK_KP_0
#define PX_KEY_NATIVE_NUMPAD1      XK_KP_1
#define PX_KEY_NATIVE_NUMPAD2      XK_KP_2
#define PX_KEY_NATIVE_NUMPAD3      XK_KP_3
#define PX_KEY_NATIVE_NUMPAD4      XK_KP_4
#define PX_KEY_NATIVE_NUMPAD5      XK_KP_5
#define PX_KEY_NATIVE_NUMPAD6      XK_KP_6
#define PX_KEY_NATIVE_NUMPAD7      XK_KP_7
#define PX_KEY_NATIVE_NUMPAD8      XK_KP_8
#define PX_KEY_NATIVE_NUMPAD9      XK_KP_9
#define PX_KEY_NATIVE_MULTIPLY     XK_KP_Multiply
#define PX_KEY_NATIVE_ADD          XK_KP_Add
#define PX_KEY_NATIVE_SEPARATOR    XK_minus //XK_KP_Separator
#define PX_KEY_NATIVE_SUBTRACT     XK_KP_Subtract
#define PX_KEY_NATIVE_DECIMAL      XK_KP_Decimal
#define PX_KEY_NATIVE_DIVIDE       XK_KP_Divide
#define PX_KEY_NATIVE_F1           XK_F1
#define PX_KEY_NATIVE_F2           XK_F2
#define PX_KEY_NATIVE_F3           XK_F3
#define PX_KEY_NATIVE_F4           XK_F4
#define PX_KEY_NATIVE_F5           XK_F5
#define PX_KEY_NATIVE_F6           XK_F6
#define PX_KEY_NATIVE_F7           XK_F7
#define PX_KEY_NATIVE_F8           XK_F8
#define PX_KEY_NATIVE_F9           XK_F9
#define PX_KEY_NATIVE_F10          XK_F10
#define PX_KEY_NATIVE_F11          XK_F11
#define PX_KEY_NATIVE_F12          XK_F12
#define PX_KEY_NATIVE_DELETE       XK_Delete
#define PX_KEY_NATIVE_NUMLOCK      XK_Num_Lock
#define PX_KEY_NATIVE_SCROLLLOCK   XK_Scroll_Lock
#define PX_KEY_NATIVE_PRINTSCREEN  0xfd1d
#define PX_KEY_NATIVE_INSERT       XK_Insert
#define PX_KEY_NATIVE_HELP         XK_Help
#define PX_KEY_NATIVE_BACKQUOTE    XK_quoteleft
#define PX_KEY_NATIVE_QUOTE        XK_quoteright

#endif // PX_WINDOW_NATIVE_GLUT_H
