/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

// pxWindowNativeGlut.h

#ifndef PX_WINDOW_NATIVE_GLUT_H
#define PX_WINDOW_NATIVE_GLUT_H

#include <stdio.h>
#ifndef WIN32
#include <sys/mman.h>
#endif
#include <cstring>
#include <vector>
#include <iostream>
//#include <linux/input.h>
#include <time.h>
#include <inttypes.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif

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
pxWindowNative(): mTimerFPS(0),/*, mLastWidth(-1), mLastHeight(-1),
                                  mResizeFlag(false), mLastAnimationTime(0.0),*/
    mVisible(false),
    mMouseEntered(false),
    mMouseDown(false),
    mGlutWindowId(0)
  {}

  virtual ~pxWindowNative();

  // Contract between pxEventLoopNative and this class
  static void runEventLoop();
  static void exitEventLoop();

  static void runEventLoopOnce();

  static std::vector<pxWindowNative*> getNativeWindows(){return mWindowVector;}

  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags) = 0;
  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags) = 0;

  virtual void onScrollWheel(float dx, float dy) = 0;
  virtual void onMouseMove(int32_t x, int32_t y) = 0;

  virtual void onMouseEnter() = 0;
  virtual void onMouseLeave() = 0;

  virtual void onKeyDown(uint32_t keycode, uint32_t flags) = 0;
  virtual void onKeyUp(uint32_t keycode, uint32_t flags) = 0;
  virtual void onChar(uint32_t c) = 0;

  virtual void onSize(int32_t w, int32_t h) = 0;

  void animateAndRender();

  void blit(pxBuffer& b, int32_t dstLeft, int32_t dstTop, 
            int32_t dstWidth, int32_t dstHeight,
            int32_t srcLeft, int32_t srcTop);

protected:
  // These are only invoked internally
  virtual void onCreate() = 0;
  virtual void onCloseRequest() = 0;
  virtual void onClose() = 0;
  virtual void onDraw(pxSurfaceNative surface) = 0;
  virtual void onAnimationTimer() = 0;

  // try to get rid of
  void onAnimationTimerInternal();
  void invalidateRectInternal(pxRect *r);

#if 0
  double getLastAnimationTime();
  void setLastAnimationTime(double time);
#endif
  void drawFrame();


  static pxWindowNative* getWindowFromGlutID(int id);
  // Glut callbacks
  static void onGlutReshape(int width, int height);
  static void onGlutClose();
  static void onGlutTimer(int v);
  static void onGlutDisplay();
  static void onGlutMouse(int button, int state, int x, int y);
  static void onGlutScrollWheel(int button, int dir, int x, int y);
  static void onGlutMouseMotion(int x, int y);
  static void onGlutMousePassiveMotion(int x, int y);
  static void onGlutKeyboard(unsigned char key, int x, int y);
  static void onGlutKeyboardSpecial(int key, int x, int y);
  static void onGlutEntry(int state);

  displayRef mDisplayRef;

  int mTimerFPS;
#if 0
  int mLastWidth, mLastHeight;
  bool mResizeFlag;
  double mLastAnimationTime;
#endif
  bool mVisible;
  bool mMouseEntered;
  bool mMouseDown;

  //timer variables
#if 0
  static bool mEventLoopTimerStarted;
  static float mEventLoopInterval;
#endif
  //static timer_t mRenderTimerId;

  void createGlutWindow(int left, int top, int width, int height);
  void cleanupGlutWindow();

  int mGlutWindowId;

  static void registerWindow(pxWindowNative* p);
  static void unregisterWindow(pxWindowNative* p); //call this method somewhere
  static std::vector<pxWindowNative*> mWindowVector;
};

// Key Codes
#define PX_KEY_NATIVE_ENTER        13
#define PX_KEY_NATIVE_BACKSPACE    8
#define PX_KEY_NATIVE_TAB          9
#define PX_KEY_NATIVE_CANCEL       10001 //TODO
#define PX_KEY_NATIVE_CLEAR        10002 //TODO
#define PX_KEY_NATIVE_SHIFT        10003 //TODO - special
#define PX_KEY_NATIVE_SHIFT_LEFT   10003 //TODO - special
#define PX_KEY_NATIVE_SHIFT_RIGHT  10003 //TODO - special
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

#endif // PX_WINDOW_NATIVE_GLUT_H
