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

#include <windows.h>
#include "../pxRect.h"
#include "pxOffscreenNative.h"

struct pxEventNativeDesc
{
    HWND    wnd;
    UINT    msg;
    WPARAM  wParam;
    LPARAM  lParam;
};

#ifndef WINCE
#define PX_DEF_WINDOW_STYLE WS_OVERLAPPEDWINDOW
#else
#define PX_DEF_WINDOW_STYLE WS_VISIBLE
#endif

typedef pxEventNativeDesc& pxEventNative;


class pxWindowNative
{
public:
    pxWindowNative(): mWindow(NULL), mTimerId(NULL), mAnimationFPS(0), mTrackMouse(false) {}
    virtual ~pxWindowNative();

    pxError initNative(HWND parent, int left, int top, int width, int height, DWORD style = PX_DEF_WINDOW_STYLE, DWORD styleEx = 0);

    pxError setAnimationFPS(long fps);

    void sendSynchronizedMessage(char* messageName, void* p1);

    void size(int& width, int& height);
    // void frameSize(...);


protected:
    virtual void onCreate() = 0;

    virtual void onCloseRequest() = 0;
    virtual void onClose() = 0;

    virtual void onSize(int w, int h) = 0;

    virtual void onMouseDown(int x, int y, uint32_t flags) = 0;
    virtual void onMouseUp(int x, int y, uint32_t flags) = 0;

    virtual void onMouseMove(int x, int y) = 0;
    virtual void onScrollWheel(float dx, float dy) = 0;

    virtual void onKeyDown(uint32_t keycode, uint32_t flags) = 0;
    virtual void onKeyUp(uint32_t keycode, uint32_t flags) = 0;
    virtual void onChar(uint32_t c) = 0;

    virtual void onDraw(pxSurfaceNative surface) = 0;

    virtual void onAnimationTimer() = 0;

    // Windows only for now

    virtual void onMouseLeave() {}
    virtual void onSynchronizedMessage(char* messageName, void* p1) {}

    // Windows only for now
    // This method will get invoked before calling any of the other event
    // handlers.  If consumed is set to true then no other event handlers will
    // be called; otherwise they will be invoked as usual
    virtual void onEvent(pxEventNative event, bool& consumed) {}

    static LRESULT __stdcall windowProc(HWND hWnd, UINT msg, 
            WPARAM wParam, LPARAM lParam);

public: // Add accessors
    HWND mWindow;
    UINT_PTR mTimerId;
    long mAnimationFPS;
	bool mTrackMouse;
};

// Key Codes

#define PX_KEY_NATIVE_ENTER        VK_RETURN
#define PX_KEY_NATIVE_BACKSPACE    VK_BACK
#define PX_KEY_NATIVE_TAB          VK_TAB
#define PX_KEY_NATIVE_CANCEL       VK_CANCEL
#define PX_KEY_NATIVE_CLEAR        VK_CLEAR
#define PX_KEY_NATIVE_SHIFT        VK_SHIFT
#define PX_KEY_NATIVE_CONTROL      VK_CONTROL
#define PX_KEY_NATIVE_ALT          0x12
#define PX_KEY_NATIVE_PAUSE        VK_PAUSE
#define PX_KEY_NATIVE_CAPSLOCK     0x14
#define PX_KEY_NATIVE_ESCAPE       VK_ESCAPE
#define PX_KEY_NATIVE_SPACE        VK_SPACE
#define PX_KEY_NATIVE_PAGEUP       VK_PRIOR
#define PX_KEY_NATIVE_PAGEDOWN     VK_NEXT
#define PX_KEY_NATIVE_END          VK_END
#define PX_KEY_NATIVE_HOME         VK_HOME
#define PX_KEY_NATIVE_LEFT         VK_LEFT
#define PX_KEY_NATIVE_UP           VK_UP
#define PX_KEY_NATIVE_RIGHT        VK_RIGHT
#define PX_KEY_NATIVE_DOWN         VK_DOWN
#define PX_KEY_NATIVE_COMMA        0xBC
#define PX_KEY_NATIVE_PERIOD       0xBE
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
#define PX_KEY_NATIVE_SEMICOLON    0xBA
#define PX_KEY_NATIVE_EQUALS       0xBB
#define PX_KEY_NATIVE_A            'A'
#define PX_KEY_NATIVE_B            'B'
#define PX_KEY_NATIVE_C            'C'
#define PX_KEY_NATIVE_D            'D'
#define PX_KEY_NATIVE_E            'E'
#define PX_KEY_NATIVE_F            'F'
#define PX_KEY_NATIVE_G            'G'
#define PX_KEY_NATIVE_H            'H'
#define PX_KEY_NATIVE_I            'I'
#define PX_KEY_NATIVE_J            'J'
#define PX_KEY_NATIVE_K            'K'
#define PX_KEY_NATIVE_L            'L'
#define PX_KEY_NATIVE_M            'M'
#define PX_KEY_NATIVE_N            'N'
#define PX_KEY_NATIVE_O            'O'
#define PX_KEY_NATIVE_P            'P'
#define PX_KEY_NATIVE_Q            'Q'
#define PX_KEY_NATIVE_R            'R'
#define PX_KEY_NATIVE_S            'S'
#define PX_KEY_NATIVE_T            'T'
#define PX_KEY_NATIVE_U            'U'
#define PX_KEY_NATIVE_V            'V'
#define PX_KEY_NATIVE_W            'W'
#define PX_KEY_NATIVE_X            'X'
#define PX_KEY_NATIVE_Y            'Y'
#define PX_KEY_NATIVE_Z            'Z'
#define PX_KEY_NATIVE_OPENBRACKET  0xDB
#define PX_KEY_NATIVE_BACKSLASH    0xDC
#define PX_KEY_NATIVE_CLOSEBRACKET 0xDD
#define PX_KEY_NATIVE_NUMPAD0      VK_NUMPAD0
#define PX_KEY_NATIVE_NUMPAD1      VK_NUMPAD1
#define PX_KEY_NATIVE_NUMPAD2      VK_NUMPAD2
#define PX_KEY_NATIVE_NUMPAD3      VK_NUMPAD3
#define PX_KEY_NATIVE_NUMPAD4      VK_NUMPAD4
#define PX_KEY_NATIVE_NUMPAD5      VK_NUMPAD5
#define PX_KEY_NATIVE_NUMPAD6      VK_NUMPAD6
#define PX_KEY_NATIVE_NUMPAD7      VK_NUMPAD7
#define PX_KEY_NATIVE_NUMPAD8      VK_NUMPAD8
#define PX_KEY_NATIVE_NUMPAD9      VK_NUMPAD9
#define PX_KEY_NATIVE_MULTIPLY     VK_MULTIPLY
#define PX_KEY_NATIVE_ADD          VK_ADD
#define PX_KEY_NATIVE_SEPARATOR    0xBD
#define PX_KEY_NATIVE_SUBTRACT     VK_SUBTRACT
#define PX_KEY_NATIVE_DECIMAL      VK_DECIMAL
#define PX_KEY_NATIVE_DIVIDE       VK_DIVIDE
#define PX_KEY_NATIVE_F1           VK_F1
#define PX_KEY_NATIVE_F2           VK_F2
#define PX_KEY_NATIVE_F3           VK_F3
#define PX_KEY_NATIVE_F4           VK_F4
#define PX_KEY_NATIVE_F5           VK_F5
#define PX_KEY_NATIVE_F6           VK_F6
#define PX_KEY_NATIVE_F7           VK_F7
#define PX_KEY_NATIVE_F8           VK_F8
#define PX_KEY_NATIVE_F9           VK_F9
#define PX_KEY_NATIVE_F10          VK_F10
#define PX_KEY_NATIVE_F11          VK_F11
#define PX_KEY_NATIVE_F12          VK_F12
#define PX_KEY_NATIVE_DELETE       VK_DELETE
#define PX_KEY_NATIVE_NUMLOCK      VK_NUMLOCK
#define PX_KEY_NATIVE_SCROLLLOCK   VK_SCROLL
#define PX_KEY_NATIVE_PRINTSCREEN  0x2C
#define PX_KEY_NATIVE_INSERT       VK_INSERT
#define PX_KEY_NATIVE_HELP         VK_HELP
#define PX_KEY_NATIVE_BACKQUOTE    0xC0
#define PX_KEY_NATIVE_QUOTE        0xDE

struct synchronizedMessage
{
	char* messageName;
	void* p1;
};
#endif
