/*
 * Copyright (C) 2019 Metrological
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PX_WPE_COMPOSITOR_H
#define PX_WPE_COMPOSITOR_H

#include <WPEFramework/core/core.h>
#include <WPEFramework/compositor/Client.h>
#include <cstdlib>
#include <sstream>

#include "../pxKeycodes.h"

template <class T>
inline std::string to_string(const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

namespace WPEFramework {

class Keyboard : public WPEFramework::Compositor::IDisplay::IKeyboard
{
private:
    Keyboard(const Keyboard&) = delete;
    Keyboard& operator=(const Keyboard&) = delete;

public:
    Keyboard() : mShiftPressed(false), mAltPressed(false), mCtrlPressed(false) {}
    virtual ~Keyboard() {}

public:
    virtual void AddRef() const {}
    virtual uint32_t Release() const { return 0; }
    virtual void KeyMap(const char* /*information*/, const uint16_t /*size*/) {}
    virtual void Key(const uint32_t /*key*/,
            const IKeyboard::state /*action*/, const uint32_t /*time*/) {}
    virtual void Modifiers(uint32_t depressedMods,
            uint32_t latchedMods, uint32_t lockedMods, uint32_t group);
    virtual void Repeat(int32_t /*rate*/, int32_t /*delay*/) {}
    virtual void Direct(const uint32_t code, const state action);

private:
    bool mShiftPressed;
    bool mAltPressed;
    bool mCtrlPressed;
};

class Display : public Core::Thread
{

public:
    enum DisplayStatus {
        NotInUse = 0,
        Attached = 1,
        Detached = 2
    };

public:
    Display(const Display&);
    Display& operator=(const Display&);

    Display()
    : _display(nullptr)
    , _displayStatus(DisplayStatus::Detached)
    , _graphicsSurface(nullptr)
    {
        Run();
    }

    void CreateDisplayClient()
    {
        std::string display_name = WPEFramework::Compositor::IDisplay::SuggestedName();
        if (display_name.empty()) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            time_t epoch_time = (tv.tv_sec * 1000) + (tv.tv_usec/1000);
            display_name = "Spark" + to_string(epoch_time);
        }
        _display = WPEFramework::Compositor::IDisplay::Instance(display_name);

        const char* width_str(std::getenv("SCREEN_WIDTH"));
        uint32_t width = (width_str == nullptr ? 1280 : atoi(width_str));
        const char* height_str(std::getenv("SCREEN_HEIGHT"));
        uint32_t height = (height_str == nullptr ? 720 : atoi(height_str));

        _graphicsSurface = _display->Create(display_name, width, height);
        printf("%s:%s:%d\n", __FILE__, __func__, __LINE__);
        _graphicsSurface->Keyboard(&_keyboardHandle);
    }

    ~Display()
    {
        Stop();
        Wait(Thread::BLOCKED | Thread::STOPPED, Core::infinite);
    };

    static Display* Instance()
    {
        static Display myDisplay;
        myDisplay.AddRef();
        myDisplay._displayStatus = DisplayStatus::Attached;
        printf("%s:%s:%d\n", __FILE__, __func__, __LINE__);
        return (&myDisplay);
    }
    void ReleaseDisplayClient()
    {
        _displayStatus = DisplayStatus::Detached;

        _display->Release();
        _graphicsSurface = nullptr;
    }
    EGLNativeDisplayType Native()
    {
        return _display->Native();
    }
    WPEFramework::Compositor::IDisplay::ISurface* GetGraphicsSurface()
    {
        return _graphicsSurface;
    }
    int FileDescriptor() {
        return _display->FileDescriptor();
    }
    void AddRef() const
    {
        if (++_refCount == 1)
        {
            const_cast<Display*>(this)->CreateDisplayClient();
        }
    }
    uint32_t Release() const
    {
        if (--_refCount == 0)
        {
             const_cast<Display*>(this)->ReleaseDisplayClient();
        }
        return 0;
    }

private:
    virtual uint32_t Worker()
    {
       while (IsRunning() == true)
       {
           if (_displayStatus == DisplayStatus::Attached)
           {
               _display->Process(1);
           }
       }
       return (WPEFramework::Core::infinite);
    }

private:
    Keyboard _keyboardHandle;
    WPEFramework::Compositor::IDisplay* _display;
    DisplayStatus _displayStatus;
    WPEFramework::Compositor::IDisplay::ISurface* _graphicsSurface;
    mutable uint32_t _refCount;
};

} // namespace WPEFramework

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
#define PX_KEY_NATIVE_PLAYPAUSE    KEY_PLAYPAUSE
#define PX_KEY_NATIVE_PLAY         KEY_PLAY
#define PX_KEY_NATIVE_FASTFORWARD  KEY_FASTFORWARD
#define PX_KEY_NATIVE_REWIND       KEY_REWIND
#define PX_KEY_NATIVE_KPENTER      KEY_KPENTER
#define PX_KEY_NATIVE_BACK         KEY_BACK
#define PX_KEY_NATIVE_MENU         KEY_MENU
#define PX_KEY_NATIVE_HOMEPAGE     KEY_HOMEPAGE

#endif // PX_WPE_COMPOSITOR_H
