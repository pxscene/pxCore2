// pxWindowNative
// PX : Portable Graphics Library
// (c) 2005 - 2007 John Robinson

#ifndef PX_WINDOW_NATIVE_H
#define PX_WINDOW_NATIVE_H

#include <Carbon/Carbon.h>

class pxWindowNative
{
public:
    pxWindowNative(): mWindowRef(NULL), mTrackingRegion(NULL), theTimer(NULL), mLastModifierState(0), mDragging(false) {}
    virtual ~pxWindowNative() {}

protected:

    virtual void onCreate() = 0;
	
	virtual void onCloseRequest() = 0;
    virtual void onClose() = 0;
	
	virtual void onSize(int w, int h) = 0;

    virtual void onMouseDown(int x, int y, unsigned long flags) = 0;
    virtual void onMouseUp(int x, int y, unsigned long flags) = 0;

    virtual void onMouseMove(int x, int y) = 0;
	
	virtual void onMouseLeave() = 0;

    virtual void onKeyDown(int keycode, unsigned long flags) = 0;
    virtual void onKeyUp(int keycode, unsigned long flags) = 0;

    virtual void onDraw(pxSurfaceNative surface) = 0;

    virtual void onAnimationTimer() = 0;
	
	void createMouseTrackingRegion();
	
	static pascal OSStatus doKeyDown(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	static pascal OSStatus doKeyUp(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	static pascal OSStatus doKeyModifierChanged(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	
	static pascal OSStatus doWindowDrawContent(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	static pascal OSStatus doWindowClosed(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	
	static pascal OSStatus doMouseDown(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	static pascal OSStatus doMouseUp(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	static pascal OSStatus doMouseMove(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	static pascal OSStatus doMouseLeave(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	
	static pascal OSStatus doWindowResizeComplete(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	static pascal OSStatus doWindowCloseRequest(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
	
	static pascal void doPeriodicTask(EventLoopTimerRef theTimer, void* userData);
	
	WindowRef mWindowRef;
	MouseTrackingRef mTrackingRegion;
	EventLoopTimerRef theTimer;	
	UInt32 mLastModifierState;
	bool mDragging;
};

// Key Codes

#define PX_KEY_NATIVE_ENTER        0x4c
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