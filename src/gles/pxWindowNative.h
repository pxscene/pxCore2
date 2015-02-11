#ifndef PX_WINDOW_NATIVE_H
#define PX_WINDOW_NATIVE_H

#include "pxBufferNative.h"
#include "pxEGLProvider.h"

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

  static pxEGLProvider* mEGLProvider;

private:
  // generic egl stuff
  EGLDisplay mEGLDisplay;
  EGLSurface mEGLSurface;
  EGLContext mEGLContext;
    
  //timer variables
  static bool mEventLoopTimerStarted;
  static float mEventLoopInterval;
  static timer_t mRenderTimerId;

};

// Key Codes - TODO: remap
#define PX_KEY_NATIVE_ENTER        13
#define PX_KEY_NATIVE_BACKSPACE    8
#define PX_KEY_NATIVE_TAB          9
#define PX_KEY_NATIVE_CANCEL       10001 //TODO
#define PX_KEY_NATIVE_CLEAR        10002 //TODO
#define PX_KEY_NATIVE_SHIFT        10003 //TODO - special
#define PX_KEY_NATIVE_CONTROL      10004 //TODO - special
#define PX_KEY_NATIVE_ALT          10005 //TODO - special
#define PX_KEY_NATIVE_PAUSE        19
#define PX_KEY_NATIVE_CAPSLOCK     20
#define PX_KEY_NATIVE_ESCAPE       27
#define PX_KEY_NATIVE_SPACE        32
#define PX_KEY_NATIVE_PAGEUP       33
#define PX_KEY_NATIVE_PAGEDOWN     34
#define PX_KEY_NATIVE_END          35
#define PX_KEY_NATIVE_HOME         36
#define PX_KEY_NATIVE_LEFT         10006
#define PX_KEY_NATIVE_UP           10007
#define PX_KEY_NATIVE_RIGHT        10008
#define PX_KEY_NATIVE_DOWN         10009
#define PX_KEY_NATIVE_COMMA        ','
#define PX_KEY_NATIVE_PERIOD       '.'
#define PX_KEY_NATIVE_SLASH        '/'
#define PX_KEY_NATIVE_ZERO         '0'
#define PX_KEY_NATIVE_ONE          '1'
#define PX_KEY_NATIVE_TWO          '2'
#define PX_KEY_NATIVE_THREE        '3'
#define PX_KEY_NATIVE_FOUR         '4'
#define PX_KEY_NATIVE_FIVE         '5'
#define PX_KEY_NATIVE_SIX          '6'
#define PX_KEY_NATIVE_SEVEN        '7'
#define PX_KEY_NATIVE_EIGHT        '8'
#define PX_KEY_NATIVE_NINE         '9'
#define PX_KEY_NATIVE_SEMICOLON    ';'
#define PX_KEY_NATIVE_EQUALS       '='
#define PX_KEY_NATIVE_A            'a'
#define PX_KEY_NATIVE_B            'b'
#define PX_KEY_NATIVE_C            'c'
#define PX_KEY_NATIVE_D            'd'
#define PX_KEY_NATIVE_E            'e'
#define PX_KEY_NATIVE_F            'f'
#define PX_KEY_NATIVE_G            'g'
#define PX_KEY_NATIVE_H            'h'
#define PX_KEY_NATIVE_I            'i'
#define PX_KEY_NATIVE_J            'j'
#define PX_KEY_NATIVE_K            'k'
#define PX_KEY_NATIVE_L            'l'
#define PX_KEY_NATIVE_M            'm'
#define PX_KEY_NATIVE_N            'n'
#define PX_KEY_NATIVE_O            'o'
#define PX_KEY_NATIVE_P            'p'
#define PX_KEY_NATIVE_Q            'q'
#define PX_KEY_NATIVE_R            'r'
#define PX_KEY_NATIVE_S            's'
#define PX_KEY_NATIVE_T            't'
#define PX_KEY_NATIVE_U            'u'
#define PX_KEY_NATIVE_V            'v'
#define PX_KEY_NATIVE_W            'w'
#define PX_KEY_NATIVE_X            'x'
#define PX_KEY_NATIVE_Y            'y'
#define PX_KEY_NATIVE_Z            'z'
#define PX_KEY_NATIVE_OPENBRACKET  '['
#define PX_KEY_NATIVE_BACKSLASH    '\\'
#define PX_KEY_NATIVE_CLOSEBRACKET ']'
#define PX_KEY_NATIVE_NUMPAD0      10010 //TODO
#define PX_KEY_NATIVE_NUMPAD1      10011 //TODO
#define PX_KEY_NATIVE_NUMPAD2      10012 //TODO
#define PX_KEY_NATIVE_NUMPAD3      10013 //TODO
#define PX_KEY_NATIVE_NUMPAD4      10014 //TODO
#define PX_KEY_NATIVE_NUMPAD5      10015 //TODO
#define PX_KEY_NATIVE_NUMPAD6      10016 //TODO
#define PX_KEY_NATIVE_NUMPAD7      10017 //TODO
#define PX_KEY_NATIVE_NUMPAD8      10018 //TODO
#define PX_KEY_NATIVE_NUMPAD9      10019 //TODO
#define PX_KEY_NATIVE_MULTIPLY     '*'
#define PX_KEY_NATIVE_ADD          '+' 
#define PX_KEY_NATIVE_SEPARATOR    '|'
#define PX_KEY_NATIVE_SUBTRACT     '-'
#define PX_KEY_NATIVE_DIVIDE       10020
#define PX_KEY_NATIVE_F1           10021
#define PX_KEY_NATIVE_F2           10022
#define PX_KEY_NATIVE_F3           10023
#define PX_KEY_NATIVE_F4           10024
#define PX_KEY_NATIVE_F5           10025
#define PX_KEY_NATIVE_F6           10026
#define PX_KEY_NATIVE_F7           10027
#define PX_KEY_NATIVE_F8           10028
#define PX_KEY_NATIVE_F9           10029
#define PX_KEY_NATIVE_F10          10030
#define PX_KEY_NATIVE_F11          10031
#define PX_KEY_NATIVE_F12          10032
#define PX_KEY_NATIVE_DELETE       10033
#define PX_KEY_NATIVE_NUMLOCK      10034
#define PX_KEY_NATIVE_SCROLLLOCK   10035 //TODO
#define PX_KEY_NATIVE_PRINTSCREEN  10036
#define PX_KEY_NATIVE_INSERT       10037 //TODO - special
#define PX_KEY_NATIVE_HELP         10038 //TODO
#define PX_KEY_NATIVE_DECIMAL      10039
#define PX_KEY_NATIVE_BACKQUOTE    '`'
#define PX_KEY_NATIVE_QUOTE        '\''

#endif
