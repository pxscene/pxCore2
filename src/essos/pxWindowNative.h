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

#include <essos.h>
#include <stdio.h>
#include <sys/mman.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <linux/input.h>
#include <time.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifndef EGL_EXT_swap_buffers_with_damage
#define EGL_EXT_swap_buffers_with_damage 1
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC)(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects);
#endif

#ifndef EGL_EXT_buffer_age
#define EGL_EXT_buffer_age 1
#define EGL_BUFFER_AGE_EXT			0x313D
#endif

// Since the lifetime of the Display should include the lifetime of all windows
// and eventloop that uses it - refcounting is utilized through this
// wrapper class.
typedef struct _essosDisplay
{
  _essosDisplay() : ctx (NULL) {}
  EssCtx *ctx;
} essosDisplay;

class displayRef
{
public:
  displayRef();
  ~displayRef();

  essosDisplay* getDisplay() const;

private:

  pxError createEssosDisplay();
  void cleanupEssosDisplay();

  static essosDisplay* mDisplay;
  static int mRefCount;
};

class pxWindowNative
{
public:
    pxWindowNative();
    virtual ~pxWindowNative();

    // Contract between pxEventLoopNative and this class
    static void runEventLoopOnce();
    static void runEventLoop();
    static void exitEventLoop();

    static std::vector<pxWindowNative*> getNativeWindows(){return mWindowVector;}

    virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags) =0;
    virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags) =0;

    virtual void onMouseMove(int32_t x, int32_t y) =0;

    virtual void onMouseLeave() =0;

    virtual void onKeyDown(uint32_t keycode, uint32_t flags) =0;
    virtual void onKeyUp(uint32_t keycode, uint32_t flags) =0;
    virtual void onChar(uint32_t c) =0;

    void onSizeUpdated(int width, int height);

    void onTouchDown(int id, int x, int y);
    void onTouchUp(int id);
    void onTouchMotion(int id, int x, int y);
    void onTouchFrame();
    
    //timer methods
    static int createAndStartEventLoopTimer(int timeoutInMilliseconds);
    static int stopAndDeleteEventLoopTimer();

    void animateAndRender();

protected:
    virtual void onCreate() = 0;

    virtual void onCloseRequest() = 0;
    virtual void onClose() = 0;
    virtual void onSize(int32_t w, int32_t h) = 0;

    virtual void onDraw(pxSurfaceNative surface) = 0;

    virtual void onAnimationTimer() = 0;	

    void onAnimationTimerInternal();

    void invalidateRectInternal(pxRect *r);
    double getLastAnimationTime();
    void setLastAnimationTime(double time);
    void drawFrame();

    void cleanupEssos();

    displayRef mDisplayRef;

    int mTimerFPS;
    int mLastWidth, mLastHeight;
    bool mResizeFlag;
    double mLastAnimationTime;
    bool mVisible;
    bool mDirty;
    
    //timer variables
    static bool mEventLoopTimerStarted;
    static float mEventLoopInterval;
    static timer_t mRenderTimerId;

    static void registerWindow(pxWindowNative* p);
    static void unregisterWindow(pxWindowNative* p); //call this method somewhere
    static std::vector<pxWindowNative*> mWindowVector;
};

// Key Codes
#define PX_KEY_NATIVE_ENTER        KEY_ENTER
#define PX_KEY_NATIVE_BACKSPACE    KEY_BACKSPACE
#define PX_KEY_NATIVE_TAB          KEY_TAB
#define PX_KEY_NATIVE_CANCEL       KEY_CANCEL
#define PX_KEY_NATIVE_CLEAR        KEY_CLEAR
#define PX_KEY_NATIVE_SHIFT        KEY_RIGHTSHIFT
#define PX_KEY_NATIVE_SHIFT_LEFT   KEY_LEFTSHIFT
#define PX_KEY_NATIVE_CONTROL      KEY_RIGHTCTRL
#define PX_KEY_NATIVE_CONTROL_LEFT KEY_LEFTCTRL
#define PX_KEY_NATIVE_ALT          KEY_RIGHTALT
#define PX_KEY_NATIVE_ALT_LEFT     KEY_LEFTALT
#define PX_KEY_NATIVE_PAUSE        KEY_PAUSE
#define PX_KEY_NATIVE_CAPSLOCK     KEY_CAPSLOCK
#define PX_KEY_NATIVE_ESCAPE       KEY_ESC
#define PX_KEY_NATIVE_SPACE        KEY_SPACE
#define PX_KEY_NATIVE_PAGEUP       KEY_PAGEUP
#define PX_KEY_NATIVE_PAGEDOWN     KEY_PAGEDOWN
#define PX_KEY_NATIVE_END          KEY_END
#define PX_KEY_NATIVE_HOME         KEY_HOME
#define PX_KEY_NATIVE_LEFT         KEY_LEFT
#define PX_KEY_NATIVE_UP           KEY_UP
#define PX_KEY_NATIVE_RIGHT        KEY_RIGHT
#define PX_KEY_NATIVE_DOWN         KEY_DOWN
#define PX_KEY_NATIVE_COMMA        KEY_COMMA
#define PX_KEY_NATIVE_PERIOD       KEY_DOT
#define PX_KEY_NATIVE_SLASH        KEY_SLASH
#define PX_KEY_NATIVE_ZERO         KEY_0
#define PX_KEY_NATIVE_ONE          KEY_1
#define PX_KEY_NATIVE_TWO          KEY_2
#define PX_KEY_NATIVE_THREE        KEY_3
#define PX_KEY_NATIVE_FOUR         KEY_4
#define PX_KEY_NATIVE_FIVE         KEY_5
#define PX_KEY_NATIVE_SIX          KEY_6
#define PX_KEY_NATIVE_SEVEN        KEY_7
#define PX_KEY_NATIVE_EIGHT        KEY_8
#define PX_KEY_NATIVE_NINE         KEY_9
#define PX_KEY_NATIVE_SEMICOLON    KEY_SEMICOLON
#define PX_KEY_NATIVE_EQUALS       KEY_EQUAL
#define PX_KEY_NATIVE_A            KEY_A
#define PX_KEY_NATIVE_B            KEY_B
#define PX_KEY_NATIVE_C            KEY_C
#define PX_KEY_NATIVE_D            KEY_D
#define PX_KEY_NATIVE_E            KEY_E
#define PX_KEY_NATIVE_F            KEY_F
#define PX_KEY_NATIVE_G            KEY_G
#define PX_KEY_NATIVE_H            KEY_H
#define PX_KEY_NATIVE_I            KEY_I
#define PX_KEY_NATIVE_J            KEY_J
#define PX_KEY_NATIVE_K            KEY_K
#define PX_KEY_NATIVE_L            KEY_L
#define PX_KEY_NATIVE_M            KEY_M
#define PX_KEY_NATIVE_N            KEY_N
#define PX_KEY_NATIVE_O            KEY_O
#define PX_KEY_NATIVE_P            KEY_P
#define PX_KEY_NATIVE_Q            KEY_Q
#define PX_KEY_NATIVE_R            KEY_R
#define PX_KEY_NATIVE_S            KEY_S
#define PX_KEY_NATIVE_T            KEY_T
#define PX_KEY_NATIVE_U            KEY_U
#define PX_KEY_NATIVE_V            KEY_V
#define PX_KEY_NATIVE_W            KEY_W
#define PX_KEY_NATIVE_X            KEY_X
#define PX_KEY_NATIVE_Y            KEY_Y
#define PX_KEY_NATIVE_Z            KEY_Z
#define PX_KEY_NATIVE_OPENBRACKET  KEY_LEFTBRACE
#define PX_KEY_NATIVE_BACKSLASH    KEY_BACKSLASH
#define PX_KEY_NATIVE_CLOSEBRACKET KEY_RIGHTBRACE
#define PX_KEY_NATIVE_NUMPAD0      KEY_KP0
#define PX_KEY_NATIVE_NUMPAD1      KEY_KP1
#define PX_KEY_NATIVE_NUMPAD2      KEY_KP2
#define PX_KEY_NATIVE_NUMPAD3      KEY_KP3
#define PX_KEY_NATIVE_NUMPAD4      KEY_KP4
#define PX_KEY_NATIVE_NUMPAD5      KEY_KP5
#define PX_KEY_NATIVE_NUMPAD6      KEY_KP6
#define PX_KEY_NATIVE_NUMPAD7      KEY_KP7
#define PX_KEY_NATIVE_NUMPAD8      KEY_KP8
#define PX_KEY_NATIVE_NUMPAD9      KEY_KP9
#define PX_KEY_NATIVE_MULTIPLY     KEY_KPASTERISK
#define PX_KEY_NATIVE_ADD          KEY_KPPLUS
#define PX_KEY_NATIVE_SEPARATOR    4256 //XK_KP_Separator
#define PX_KEY_NATIVE_SUBTRACT     KEY_MINUS
#define PX_KEY_NATIVE_DECIMAL      KEY_KPDOT
#define PX_KEY_NATIVE_DIVIDE       KEY_KPSLASH //todo - check this
#define PX_KEY_NATIVE_F1           KEY_F1
#define PX_KEY_NATIVE_F2           KEY_F2
#define PX_KEY_NATIVE_F3           KEY_F3
#define PX_KEY_NATIVE_F4           KEY_F4
#define PX_KEY_NATIVE_F5           KEY_F5
#define PX_KEY_NATIVE_F6           KEY_F6
#define PX_KEY_NATIVE_F7           KEY_F7
#define PX_KEY_NATIVE_F8           KEY_F8
#define PX_KEY_NATIVE_F9           KEY_F9
#define PX_KEY_NATIVE_F10          KEY_F10
#define PX_KEY_NATIVE_F11          KEY_F11
#define PX_KEY_NATIVE_F12          KEY_F12
#define PX_KEY_NATIVE_DELETE       KEY_DELETE
#define PX_KEY_NATIVE_NUMLOCK      KEY_NUMLOCK
#define PX_KEY_NATIVE_SCROLLLOCK   KEY_SCROLLLOCK
#define PX_KEY_NATIVE_PRINTSCREEN  KEY_PRINT
#define PX_KEY_NATIVE_INSERT       KEY_INSERT
#define PX_KEY_NATIVE_HELP         KEY_HELP
#define PX_KEY_NATIVE_BACKQUOTE    KEY_GRAVE
#define PX_KEY_NATIVE_QUOTE        KEY_APOSTROPHE

#endif
