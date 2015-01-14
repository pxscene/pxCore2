// pxCore CopyRight 2005-2006 John Robinson
// Portable Framebuffer and Windowing Library
// pxWindowNative.h

#ifndef PX_WINDOW_NATIVE_H
#define PX_WINDOW_NATIVE_H

#include <wayland-client.h>
#include <stdio.h>
#include <sys/mman.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <linux/input.h>
using namespace std;



// Since the lifetime of the Display should include the lifetime of all windows
// and eventloop that uses it - refcounting is utilized through this
// wrapper class.
typedef struct _waylandDisplay {

    _waylandDisplay() : display(NULL), registry(NULL), compositor(NULL), shm(NULL),
        shell(NULL), seat(NULL), pointer(NULL), mousePositionX(0), mousePositionY(0) {}

    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shm *shm;
    struct wl_shell *shell;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;
    int mousePositionX;
    int mousePositionY;
} waylandDisplay;

typedef struct _waylandBuffer {
    _waylandBuffer() : buffer (NULL), shm_data(NULL), busy(0){}
    struct wl_buffer *buffer;
    void *shm_data;
    int busy;
} waylandBuffer;

class displayRef
{
public:
    displayRef()
    {
        if (mRefCount == 0)
        {
            mRefCount++;
            createWaylandDisplay();
        }
        else
        {
            mRefCount++;
        }
    }

    ~displayRef()
    {
        mRefCount--;
        if (mRefCount == 0)
        {
            cleanupWaylandDisplay();
        }
    }

    waylandDisplay* getDisplay() const { return mDisplay; }

    static struct wl_registry_listener mWaylandRegistryListener;
    static struct wl_pointer_listener mWaylandPointerListener;
    static struct wl_keyboard_listener mWaylandKeyboardListener;

private:

    pxError createWaylandDisplay();
    void cleanupWaylandDisplay();

    static waylandDisplay* mDisplay;
    static int mRefCount;
};

class pxWindowNative
{
public:
pxWindowNative(): mTimerFPS(0), mLastWidth(-1), mLastHeight(-1),
    mResizeFlag(false), mLastAnimationTime(0.0), mVisible(false),
    mWaylandSurface(NULL), mWaylandBuffer(), waylandBufferIndex(0), mFrameCallback(NULL)
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

    void drawFrame(wl_callback* callback, pxRect *rect = NULL);

protected:
    virtual void onCreate() = 0;

    virtual void onCloseRequest() = 0;
    virtual void onClose() = 0;

    virtual void onSize(int w, int h) = 0;

    virtual void onDraw(pxSurfaceNative surface) = 0;

    virtual void onAnimationTimer() = 0;	

    void onAnimationTimerInternal();

    void invalidateRectInternal(pxRect *r);


    //wayland helper methods
    struct wl_shell_surface* createWaylandSurface();
    void cleanupWaylandData();

    int set_cloexec_or_close(int fd);
    int create_tmpfile_cloexec(char *tmpname);
    int os_create_anonymous_file(off_t size);
    int createShmBuffer(waylandDisplay *display, waylandBuffer *buffer, int width, int height, uint32_t format);
    waylandBuffer* nextBuffer();


    displayRef mDisplayRef;

    int mTimerFPS;
    int mLastWidth, mLastHeight;
    bool mResizeFlag;
    double mLastAnimationTime;
    bool mVisible;

    //wayland stuff
    struct wl_shell_surface *mWaylandSurface;

    waylandBuffer mWaylandBuffer[2];
    int waylandBufferIndex;

    wl_callback* mFrameCallback;

    static void registerWindow(pxWindowNative* p);
    static void unregisterWindow(pxWindowNative* p); //call this method somewhere
    static vector<pxWindowNative*> mWindowVector;
};

// Key Codes
#define PX_KEY_ENTER        KEY_ENTER
#define PX_KEY_BACKSPACE    KEY_BACKSPACE
#define PX_KEY_TAB          KEY_TAB
#define PX_KEY_CANCEL       KEY_CANCEL
#define PX_KEY_CLEAR        KEY_CLEAR
#define PX_KEY_SHIFT        KEY_RIGHTSHIFT
#define PX_KEY_SHIFT_LEFT   KEY_LEFTSHIFT
#define PX_KEY_CONTROL      KEY_RIGHTCTRL
#define PX_KEY_CONTROL_LEFT KEY_LEFTCTRL
#define PX_KEY_ALT          KEY_RIGHTALT
#define PX_KEY_ALT_LEFT     KEY_LEFTALT
#define PX_KEY_PAUSE        KEY_PAUSE
#define PX_KEY_CAPSLOCK     KEY_CAPSLOCK
#define PX_KEY_ESCAPE       KEY_ESC
#define PX_KEY_SPACE        KEY_SPACE
#define PX_KEY_PAGEUP       KEY_PAGEUP
#define PX_KEY_PAGEDOWN     KEY_PAGEDOWN
#define PX_KEY_END          KEY_END
#define PX_KEY_HOME         KEY_HOME
#define PX_KEY_LEFT         KEY_LEFT
#define PX_KEY_UP           KEY_UP
#define PX_KEY_RIGHT        KEY_RIGHT
#define PX_KEY_DOWN         KEY_DOWN
#define PX_KEY_COMMA        KEY_COMMA
#define PX_KEY_PERIOD       KEY_DOT
#define PX_KEY_SLASH        KEY_SLASH
#define PX_KEY_ZERO         KEY_0
#define PX_KEY_ONE          KEY_1
#define PX_KEY_TWO          KEY_2
#define PX_KEY_THREE        KEY_3
#define PX_KEY_FOUR         KEY_4
#define PX_KEY_FIVE         KEY_5
#define PX_KEY_SIX          KEY_6
#define PX_KEY_SEVEN        KEY_7
#define PX_KEY_EIGHT        KEY_8
#define PX_KEY_NINE         KEY_9
#define PX_KEY_SEMICOLON    KEY_SEMICOLON
#define PX_KEY_EQUALS       KEY_EQUAL
#define PX_KEY_A            KEY_A
#define PX_KEY_B            KEY_B
#define PX_KEY_C            KEY_C
#define PX_KEY_D            KEY_D
#define PX_KEY_E            KEY_E
#define PX_KEY_F            KEY_F
#define PX_KEY_G            KEY_G
#define PX_KEY_H            KEY_H
#define PX_KEY_I            KEY_I
#define PX_KEY_J            KEY_J
#define PX_KEY_K            KEY_K
#define PX_KEY_L            KEY_L
#define PX_KEY_M            KEY_M
#define PX_KEY_N            KEY_N
#define PX_KEY_O            KEY_O
#define PX_KEY_P            KEY_P
#define PX_KEY_Q            KEY_Q
#define PX_KEY_R            KEY_R
#define PX_KEY_S            KEY_S
#define PX_KEY_T            KEY_T
#define PX_KEY_U            KEY_U
#define PX_KEY_V            KEY_V
#define PX_KEY_W            KEY_W
#define PX_KEY_X            KEY_X
#define PX_KEY_Y            KEY_Y
#define PX_KEY_Z            KEY_Z
#define PX_KEY_OPENBRACKET  KEY_LEFTBRACE
#define PX_KEY_BACKSLASH    KEY_BACKSLASH
#define PX_KEY_CLOSEBRACKET KEY_RIGHTBRACE
#define PX_KEY_NUMPAD0      KEY_KP0
#define PX_KEY_NUMPAD1      KEY_KP1
#define PX_KEY_NUMPAD2      KEY_KP2
#define PX_KEY_NUMPAD3      KEY_KP3
#define PX_KEY_NUMPAD4      KEY_KP4
#define PX_KEY_NUMPAD5      KEY_KP5
#define PX_KEY_NUMPAD6      KEY_KP6
#define PX_KEY_NUMPAD7      KEY_KP7
#define PX_KEY_NUMPAD8      KEY_KP8
#define PX_KEY_NUMPAD9      KEY_KP9
#define PX_KEY_MULTIPLY     KEY_KPASTERISK
#define PX_KEY_ADD          KEY_KPPLUS
#define PX_KEY_SEPARATOR    4256 //XK_KP_Separator
#define PX_KEY_SUBTRACT     KEY_MINUS
#define PX_KEY_DECIMAL      KEY_KPDOT
#define PX_KEY_DIVIDE       KEY_KPSLASH //todo - check this
#define PX_KEY_F1           KEY_F1
#define PX_KEY_F2           KEY_F2
#define PX_KEY_F3           KEY_F3
#define PX_KEY_F4           KEY_F4
#define PX_KEY_F5           KEY_F5
#define PX_KEY_F6           KEY_F6
#define PX_KEY_F7           KEY_F7
#define PX_KEY_F8           KEY_F8
#define PX_KEY_F9           KEY_F9
#define PX_KEY_F10          KEY_F10
#define PX_KEY_F11          KEY_F11
#define PX_KEY_F12          KEY_F12
#define PX_KEY_DELETE       KEY_DELETE
#define PX_KEY_NUMLOCK      KEY_NUMLOCK
#define PX_KEY_SCROLLLOCK   KEY_SCROLLLOCK
#define PX_KEY_PRINTSCREEN  KEY_PRINT
#define PX_KEY_INSERT       KEY_INSERT
#define PX_KEY_HELP         KEY_HELP
#define PX_KEY_BACKQUOTE    KEY_GRAVE
#define PX_KEY_QUOTE        KEY_APOSTROPHE

#endif
