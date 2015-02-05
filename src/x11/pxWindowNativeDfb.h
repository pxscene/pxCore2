// pxCore CopyRight 2005-2006 John Robinson
// Portable Framebuffer and Windowing Library
// pxWindowNativeDfb.h

#ifndef PX_WINDOW_NATIVE_DFB_H
#define PX_WINDOW_NATIVE_DFB_H

//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <X11/keysymdef.h>
//#include <X11/Xatom.h>

#include <stdio.h>
#include <sys/mman.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <linux/input.h>
#include <time.h>

#include <directfb.h>

using namespace std;



// Since the lifetime of the Display should include the lifetime of all windows
// and eventloop that uses it - refcounting is utilized through this
// wrapper class.
typedef struct _dfbDisplay
{
    _dfbDisplay() {}
}
dfbDisplay;

class displayRef
{
public:
    displayRef();
    ~displayRef();

    dfbDisplay* getDisplay() const;

private:

    pxError createDfbDisplay();
    void cleanupDfbDisplay();

    static dfbDisplay* mDisplay;
    static int mRefCount;
};

class pxWindowNative
{
public:
pxWindowNative(): mTimerFPS(0), mLastWidth(-1), mLastHeight(-1),
    mResizeFlag(false), mLastAnimationTime(0.0), mVisible(false), 
    mDfbWindowId(0)
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

    int    mTimerFPS;

    int    mLastWidth;
    int    mLastHeight;

    bool   mResizeFlag;
    double mLastAnimationTime;
    bool   mVisible;
    
    //timer variables
    static bool    mEventLoopTimerStarted;
    static float   mEventLoopInterval;
    static timer_t mRenderTimerId;
    
    void createDfbWindow(int left, int top, int width, int height);
    void cleanupDfbWindow();
    
    int mDfbWindowId;

    static void registerWindow(pxWindowNative* p);
    static void unregisterWindow(pxWindowNative* p); //call this method somewhere
    static vector<pxWindowNative*> mWindowVector;
};

// Key Codes
#define PX_KEY_ENTER        DIKS_RETURN  // **
#define PX_KEY_BACKSPACE    DIKI_BACKSPACE
#define PX_KEY_TAB          DIKI_TAB
#define PX_KEY_CANCEL       DIKS_CANCEL  // **
#define PX_KEY_CLEAR        DIKS_CLEAR   // **
#define PX_KEY_SHIFT        DIKI_SHIFT_L
#define PX_KEY_CONTROL      DIKI_CONTROL_L
#define PX_KEY_ALT          DIKI_ALT_L
#define PX_KEY_PAUSE        DIKI_PAUSE
#define PX_KEY_CAPSLOCK     DIKI_CAPS_LOCK
#define PX_KEY_ESCAPE       DIKI_ESCAPE
#define PX_KEY_SPACE        DIKI_SPACE
#define PX_KEY_PAGEUP       DIKI_PAGE_UP
#define PX_KEY_PAGEDOWN     DIKI_PAGE_DOWN
#define PX_KEY_END          DIKI_END
#define PX_KEY_HOME         DIKI_HOME
#define PX_KEY_LEFT         DIKI_LEFT
#define PX_KEY_UP           DIKI_UP
#define PX_KEY_RIGHT        DIKI_RIGHT
#define PX_KEY_DOWN         DIKI_DOWN
#define PX_KEY_COMMA        DIKI_COMMA
#define PX_KEY_PERIOD       DIKI_PERIOD
#define PX_KEY_SLASH        DIKI_SLASH
#define PX_KEY_ZERO         DIKI_0
#define PX_KEY_ONE          DIKI_1
#define PX_KEY_TWO          DIKI_2
#define PX_KEY_THREE        DIKI_3
#define PX_KEY_FOUR         DIKI_4
#define PX_KEY_FIVE         DIKI_5
#define PX_KEY_SIX          DIKI_6
#define PX_KEY_SEVEN        DIKI_7
#define PX_KEY_EIGHT        DIKI_8
#define PX_KEY_NINE         DIKI_9
#define PX_KEY_SEMICOLON    DIKI_SEMICOLON
#define PX_KEY_EQUALS       DIKI_EQUAL
#define PX_KEY_A            DIKI_A
#define PX_KEY_B            DIKI_B
#define PX_KEY_C            DIKI_C
#define PX_KEY_D            DIKI_D
#define PX_KEY_E            DIKI_E
#define PX_KEY_F            DIKI_F
#define PX_KEY_G            DIKI_G
#define PX_KEY_H            DIKI_H
#define PX_KEY_I            DIKI_I
#define PX_KEY_J            DIKI_J
#define PX_KEY_K            DIKI_K
#define PX_KEY_L            DIKI_L
#define PX_KEY_M            DIKI_M
#define PX_KEY_N            DIKI_N
#define PX_KEY_O            DIKI_O
#define PX_KEY_P            DIKI_P
#define PX_KEY_Q            DIKI_Q
#define PX_KEY_R            DIKI_R
#define PX_KEY_S            DIKI_S
#define PX_KEY_T            DIKI_T
#define PX_KEY_U            DIKI_U
#define PX_KEY_V            DIKI_V
#define PX_KEY_W            DIKI_W
#define PX_KEY_X            DIKI_X
#define PX_KEY_Y            DIKI_Y
#define PX_KEY_Z            DIKI_Z
#define PX_KEY_OPENBRACKET  DIKI_BRACKET_LEFT
#define PX_KEY_BACKSLASH    DIKI_BACKSLASH
#define PX_KEY_CLOSEBRACKET DIKI_BRACKET_RIGHT
#define PX_KEY_NUMPAD0      DIKI_KP_0
#define PX_KEY_NUMPAD1      DIKI_KP_1
#define PX_KEY_NUMPAD2      DIKI_KP_2
#define PX_KEY_NUMPAD3      DIKI_KP_3
#define PX_KEY_NUMPAD4      DIKI_KP_4
#define PX_KEY_NUMPAD5      DIKI_KP_5
#define PX_KEY_NUMPAD6      DIKI_KP_6
#define PX_KEY_NUMPAD7      DIKI_KP_7
#define PX_KEY_NUMPAD8      DIKI_KP_8
#define PX_KEY_NUMPAD9      DIKI_KP_9
#define PX_KEY_MULTIPLY     DIKI_KP_MULTIPLY
#define PX_KEY_ADD          DIKI_KP_ADD
#define PX_KEY_SEPARATOR    DIKI_MINUS
#define PX_KEY_SUBTRACT     DIKI_KP_MINUS
#define PX_KEY_DECIMAL      DIKI_KP_DECIMAL
#define PX_KEY_DIVIDE       DIKI_KP_DIVIDE
#define PX_KEY_F1           DIKI_F1
#define PX_KEY_F2           DIKI_F2
#define PX_KEY_F3           DIKI_F3
#define PX_KEY_F4           DIKI_F4
#define PX_KEY_F5           DIKI_F5
#define PX_KEY_F6           DIKI_F6
#define PX_KEY_F7           DIKI_F7
#define PX_KEY_F8           DIKI_F8
#define PX_KEY_F9           DIKI_F9
#define PX_KEY_F10          DIKI_F10
#define PX_KEY_F11          DIKI_F11
#define PX_KEY_F12          DIKI_F12
#define PX_KEY_DELETE       DIKI_DELETE
#define PX_KEY_NUMLOCK      DIKI_NUM_LOCK
#define PX_KEY_SCROLLLOCK   DIKI_SCROLL_LOCK
#define PX_KEY_PRINTSCREEN  DIKI_PRINT
#define PX_KEY_INSERT       DIKI_INSERT
#define PX_KEY_HELP         DIKI_HELP
#define PX_KEY_BACKQUOTE    DIKI_QUOTE_LEFT
#define PX_KEY_QUOTE        DIKI_QUOTE_RIGHT

#endif //PX_WINDOW_NATIVE_DFB_H


/*
 DWET_NONE	 	0x00000000
  DWET_POSITION	 	0x00000001	 	window has been moved by window manager or the application itself
  DWET_SIZE	 	0x00000002	 	window has been resized by window manager or the application itself
  DWET_CLOSE	 	0x00000004	 	closing this window has been requested only
  DWET_DESTROYED	 	0x00000008	 	window got destroyed by global deinitialization function or the application itself
  DWET_GOTFOCUS	 	0x00000010	 	window got focus
  DWET_LOSTFOCUS	 	0x00000020	 	window lost focus
  DWET_KEYDOWN	 	0x00000100	 	a key has gone down while window has focus
  DWET_KEYUP	 	0x00000200	 	a key has gone up while window has focus
  DWET_BUTTONDOWN	 	0x00010000	 	mouse button went down in the window
  DWET_BUTTONUP	 	0x00020000	 	mouse button went up in the window
  DWET_MOTION	 	0x00040000	 	mouse cursor changed its position in window
  DWET_ENTER	 	0x00080000	 	mouse cursor entered the window
  DWET_LEAVE	 	0x00100000	 	mouse cursor left the window
  DWET_WHEEL	 	0x00200000	 	mouse wheel was moved while window has focus
  DWET_POSITION_SIZE	 	DWET_POSITION | DWET_SIZE	 	initially sent to window when it's created
  DWET_ALL	 	0x003F033F
 */
