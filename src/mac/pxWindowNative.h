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

// pxWindowNative.h

#ifndef PX_WINDOW_NATIVE_H
#define PX_WINDOW_NATIVE_H

#include <string>
#include <algorithm>
#include <vector>

class pxWindowNative
{
public:
  pxWindowNative(): mWindow(NULL),mTimer(NULL)/*: mWindowRef(NULL), mTrackingRegion(NULL), theTimer(NULL), mLastModifierState(0), mDragging(false)*/ {}
  virtual ~pxWindowNative();
  
  // The Joy of ObjectiveC++
  static void _helper_onCreate(pxWindowNative* w)
  {
    if (w)
      w->onCreate();
  }
  static void _helper_onCloseRequest(pxWindowNative* w) { if (w) w->onCloseRequest(); }
  static void _helper_onClose(pxWindowNative* w)
  {
    if (w)
      w->onClose();
  }
  static void _helper_onSize(pxWindowNative* win, int32_t w, int32_t h)
  {
    if (win)
      win->onSize(w, h);
  }
  static void _helper_onMouseDown(pxWindowNative* w, int32_t x, int32_t y, uint32_t flags)
  {
    if (w)
      w->onMouseDown(x, y, flags);
  }
  static void _helper_onMouseUp(pxWindowNative* w, int32_t x, int32_t y, uint32_t flags)
  {
    if (w)
      w->onMouseUp(x, y, flags);
  }
  static void _helper_onMouseMove(pxWindowNative* w, int32_t x, int32_t y)
  {
    if (w)
      w->onMouseMove(x, y);
  }
  static void _helper_onScrollWheel(pxWindowNative* w, float dx, float dy)
  {
    if (w)
    w->onScrollWheel(dx, dy);
  }
  static void _helper_onMouseEnter(pxWindowNative* w)
  {
    if (w)
      w->onMouseEnter();
  }
  static void _helper_onMouseLeave(pxWindowNative* w)
  {
    if (w)
      w->onMouseLeave();
  }
  static void _helper_onKeyDown(pxWindowNative* w, uint32_t keycode, uint32_t flags)
  {
    if (w)
      w->onKeyDown(keycode, flags);
  }
  static void _helper_onKeyUp(pxWindowNative* w, uint32_t keycode, uint32_t flags)
  {
    if (w)
      w->onKeyUp(keycode, flags);
  }
  static void _helper_onChar(pxWindowNative* w, uint32_t c)
  {
    if (w)
      w->onChar(c);
  }
  static void _helper_onDraw(pxWindowNative* w, pxSurfaceNative surface)
  {
    if (w)
      w->onDraw(surface);
  }
  static void _helper_onAnimationTimer(pxWindowNative* w)
  {
    if (w)
      w->onAnimationTimer();
  }

  static void closeAllWindows();
  static void registerWindow(pxWindowNative*);
  static void unregisterWindow(pxWindowNative*);
  static std::vector<pxWindowNative *> sWindowVector;
protected:
  
  virtual void onCreate() = 0;
  
  virtual void onCloseRequest() = 0;
  virtual void onClose() = 0;
  
  virtual void onSize(int32_t w, int32_t h) = 0;
  
  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags) = 0;
  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags) = 0;
  
  virtual void onMouseMove(int32_t x, int32_t y) = 0;
  virtual void onScrollWheel(float /*dx*/, float /*dy*/) { /*empty*/ };
  
  virtual void onMouseEnter() = 0;
  virtual void onMouseLeave() = 0;
  
  virtual void onKeyDown(uint32_t keycode, uint32_t flags) = 0;
  virtual void onKeyUp(uint32_t keycode, uint32_t flags) = 0;
  virtual void onChar(uint32_t c) = 0;
  
  virtual void onDraw(pxSurfaceNative surface) = 0;
  
  virtual void onAnimationTimer() = 0;
  
  void* mWindow;
  void* mDelegate;
  void* mTimer;
};

// Key Codes

#define PX_KEY_NATIVE_ENTER        0x24
#define PX_KEY_NATIVE_BACKSPACE    0x33
#define PX_KEY_NATIVE_TAB          0x30
#define PX_KEY_NATIVE_CLEAR        0x47
#define PX_KEY_NATIVE_SHIFT        0x38
#define PX_KEY_NATIVE_CONTROL      0x3B
#define PX_KEY_NATIVE_ALT          0x3A
#define PX_KEY_NATIVE_PAUSE        0x71
#define PX_KEY_NATIVE_CAPSLOCK     0x39
#define PX_KEY_NATIVE_ESCAPE       0x35
#define PX_KEY_NATIVE_SPACE        0x31
#define PX_KEY_NATIVE_PAGEUP       0x74
#define PX_KEY_NATIVE_PAGEDOWN     0x79
#define PX_KEY_NATIVE_END          0x77
#define PX_KEY_NATIVE_HOME         0x73
#define PX_KEY_NATIVE_LEFT         0x7B
#define PX_KEY_NATIVE_UP           0x7E
#define PX_KEY_NATIVE_RIGHT        0x7C
#define PX_KEY_NATIVE_DOWN         0x7D
#define PX_KEY_NATIVE_COMMA        0x2B
#define PX_KEY_NATIVE_PERIOD       0x2F
#define PX_KEY_NATIVE_SLASH        0x2C
#define PX_KEY_NATIVE_ZERO         0x1D
#define PX_KEY_NATIVE_ONE          0x12
#define PX_KEY_NATIVE_TWO          0x13
#define PX_KEY_NATIVE_THREE        0x14
#define PX_KEY_NATIVE_FOUR         0x15
#define PX_KEY_NATIVE_FIVE         0x17
#define PX_KEY_NATIVE_SIX          0x16
#define PX_KEY_NATIVE_SEVEN        0x1A
#define PX_KEY_NATIVE_EIGHT        0x1C
#define PX_KEY_NATIVE_NINE         0x19
#define PX_KEY_NATIVE_SEMICOLON    0x29
#define PX_KEY_NATIVE_EQUALS       0x18
#define PX_KEY_NATIVE_A            0x00
#define PX_KEY_NATIVE_B            0x0B
#define PX_KEY_NATIVE_C            0x08
#define PX_KEY_NATIVE_D            0x02
#define PX_KEY_NATIVE_E            0x0E
#define PX_KEY_NATIVE_F            0x03
#define PX_KEY_NATIVE_G            0x05
#define PX_KEY_NATIVE_H            0x04
#define PX_KEY_NATIVE_I            0x22
#define PX_KEY_NATIVE_J            0x26
#define PX_KEY_NATIVE_K            0x28
#define PX_KEY_NATIVE_L            0x25
#define PX_KEY_NATIVE_M            0x2E
#define PX_KEY_NATIVE_N            0x2D
#define PX_KEY_NATIVE_O            0x1F
#define PX_KEY_NATIVE_P            0x23
#define PX_KEY_NATIVE_Q            0x0C
#define PX_KEY_NATIVE_R            0x0F
#define PX_KEY_NATIVE_S            0x01
#define PX_KEY_NATIVE_T            0x11
#define PX_KEY_NATIVE_U            0x20
#define PX_KEY_NATIVE_V            0x09
#define PX_KEY_NATIVE_W            0x0D
#define PX_KEY_NATIVE_X            0x07
#define PX_KEY_NATIVE_Y            0x10
#define PX_KEY_NATIVE_Z            0x06
#define PX_KEY_NATIVE_OPENBRACKET  0x21
#define PX_KEY_NATIVE_BACKSLASH    0x2A
#define PX_KEY_NATIVE_CLOSEBRACKET 0x1E
#define PX_KEY_NATIVE_NUMPAD0      0x52
#define PX_KEY_NATIVE_NUMPAD1      0x53
#define PX_KEY_NATIVE_NUMPAD2      0x54
#define PX_KEY_NATIVE_NUMPAD3      0x55
#define PX_KEY_NATIVE_NUMPAD4      0x56
#define PX_KEY_NATIVE_NUMPAD5      0x57
#define PX_KEY_NATIVE_NUMPAD6      0x58
#define PX_KEY_NATIVE_NUMPAD7      0x59
#define PX_KEY_NATIVE_NUMPAD8      0x5B
#define PX_KEY_NATIVE_NUMPAD9      0x5C
#define PX_KEY_NATIVE_SEPARATOR    0x1B
#define PX_KEY_NATIVE_ADD          0x45
#define PX_KEY_NATIVE_SUBTRACT     0x4E
#define PX_KEY_NATIVE_DECIMAL      0x41
#define PX_KEY_NATIVE_MULTIPLY     0x43
#define PX_KEY_NATIVE_DIVIDE       0x4B
#define PX_KEY_NATIVE_F1           0x7A
#define PX_KEY_NATIVE_F2           0x78
#define PX_KEY_NATIVE_F3           0x63
#define PX_KEY_NATIVE_F4           0x76
#define PX_KEY_NATIVE_F5           0x60
#define PX_KEY_NATIVE_F6           0x61
#define PX_KEY_NATIVE_F7           0x62
#define PX_KEY_NATIVE_F8           0x64
#define PX_KEY_NATIVE_F9           0x65
#define PX_KEY_NATIVE_F10          0x6D
#define PX_KEY_NATIVE_F11          0x67
#define PX_KEY_NATIVE_F12          0x6F
#define PX_KEY_NATIVE_DELETE       0x75
#define PX_KEY_NATIVE_NUMLOCK      0x47
#define PX_KEY_NATIVE_SCROLLLOCK   0x6B
#define PX_KEY_NATIVE_PRINTSCREEN  0x69
#define PX_KEY_NATIVE_INSERT       0x72
#define PX_KEY_NATIVE_BACKQUOTE    0x32
#define PX_KEY_NATIVE_QUOTE        0x27

#endif
