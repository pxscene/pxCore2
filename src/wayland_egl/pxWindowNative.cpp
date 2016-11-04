// pxCore CopyRight 2005-2006 John Robinson
// Portable Framebuffer and Windowing Library
// pxWindowNative.cpp

#include "../pxCore.h"
#include "../pxWindow.h"
#include "pxWindowNative.h"
#include "../pxTimer.h"
#include "../pxWindowUtil.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h> //todo: remove when done with assert
#include <unistd.h> //for close()
#include <fcntl.h> //for files
#include <unistd.h>
#include <signal.h>

#define WAYLAND_EGL_BUFFER_SIZE 32
#define WAYLAND_EGL_BUFFER_OPAQUE 0
#define WAYLAND_PX_CORE_FPS 30

#define MOD_SHIFT	0x01
#define MOD_ALT		0x02
#define MOD_CTRL	0x04

bool bShiftPressed = false;
bool bAltPressed = false;
bool bCtrlPressed = false;


waylandDisplay* displayRef::mDisplay = NULL;
struct wl_registry_listener displayRef::mWaylandRegistryListener;
struct wl_pointer_listener displayRef::mWaylandPointerListener;
struct wl_keyboard_listener displayRef::mWaylandKeyboardListener;
int displayRef::mRefCount = 0;
struct wl_shell_surface_listener pxWindowNative::mShellSurfaceListener;
vector<pxWindowNative*> pxWindowNative::mWindowVector;
bool pxWindowNative::mEventLoopTimerStarted = false;
float pxWindowNative::mEventLoopInterval = 1000.0 / (float)WAYLAND_PX_CORE_FPS;
timer_t pxWindowNative::mRenderTimerId;



displayRef::displayRef()
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

displayRef::~displayRef()
{
    mRefCount--;
    if (mRefCount == 0)
    {
        pxWindowNative::stopAndDeleteEventLoopTimer();
        cleanupWaylandDisplay();
    }
}

waylandDisplay* displayRef::getDisplay() const
{
    return mDisplay;
}

//begin wayland callbacks

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    displayRef dRef;
    if (strcmp(interface, wl_compositor_interface.name) == 0)
        dRef.getDisplay()->compositor = (struct wl_compositor*)wl_registry_bind(registry, name,
            &wl_compositor_interface, 1 /*version*/);
    else if (strcmp(interface, wl_shm_interface.name) == 0)
        dRef.getDisplay()->shm = (struct wl_shm*)wl_registry_bind(registry, name,
            &wl_shm_interface, 1/*version*/);
    else if (strcmp(interface, wl_shell_interface.name) == 0)
        dRef.getDisplay()->shell = (struct wl_shell*)wl_registry_bind(registry, name,
            &wl_shell_interface, 1 /*version*/);
    else if (strcmp(interface, wl_seat_interface.name) == 0)
    {
        dRef.getDisplay()->seat = (struct wl_seat*)wl_registry_bind(registry, name,
            &wl_seat_interface, 1 /*version*/);
        dRef.getDisplay()->pointer = (struct wl_pointer*)wl_seat_get_pointer(dRef.getDisplay()->seat);
        wl_pointer_add_listener(dRef.getDisplay()->pointer, &displayRef::mWaylandPointerListener,
            dRef.getDisplay());
        dRef.getDisplay()->keyboard = (struct wl_keyboard*)wl_seat_get_keyboard(dRef.getDisplay()->seat);
        wl_keyboard_add_listener(dRef.getDisplay()->keyboard, &displayRef::mWaylandKeyboardListener,
            NULL);
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
}

static void
pointer_handle_enter(void *data, struct wl_pointer *pointer,
             uint32_t serial, struct wl_surface *surface,
             wl_fixed_t sx, wl_fixed_t sy)
{
}

static void
pointer_handle_leave(void *data, struct wl_pointer *pointer,
             uint32_t serial, struct wl_surface *surface)
{
    vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
    vector<pxWindowNative*>::iterator i;
    for (i = windowVector.begin(); i < windowVector.end(); i++)
    {
        pxWindowNative* w = (*i);
        w->onMouseLeave();
    }
}

static void
pointer_handle_motion(void *data, struct wl_pointer *pointer,
              uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
    int lastMouseXPosition = wl_fixed_to_int(sx);
    int lastMouseYPosition = wl_fixed_to_int(sy);
    waylandDisplay* wDisplay = (waylandDisplay*)data;
    wDisplay->mousePositionX = lastMouseXPosition;
    wDisplay->mousePositionY = lastMouseYPosition;
    vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
    vector<pxWindowNative*>::iterator i;
    for (i = windowVector.begin(); i < windowVector.end(); i++)
    {
        pxWindowNative* w = (*i);
        w->onMouseMove(lastMouseXPosition,lastMouseYPosition);
    }
}

static void
pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
              uint32_t serial, uint32_t time, uint32_t button,
              uint32_t state)
{
    unsigned long flags;
    switch(button)
    {
        case BTN_MIDDLE: flags = PX_MIDDLEBUTTON;
        break;
        case BTN_RIGHT: flags = PX_RIGHTBUTTON;
        break;
        default: flags = PX_LEFTBUTTON;
        break;
    }

    waylandDisplay* wDisplay = (waylandDisplay*)data;
    int lastMouseXPosition = wDisplay->mousePositionX;
    int lastMouseYPosition = wDisplay->mousePositionY;

    vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
    vector<pxWindowNative*>::iterator i;
    for (i = windowVector.begin(); i < windowVector.end(); i++)
    {
        pxWindowNative* w = (*i);
        if (state == WL_POINTER_BUTTON_STATE_PRESSED)
        {
            w->onMouseDown(lastMouseXPosition, lastMouseYPosition, flags);
        }
        else
        {
            w->onMouseUp(lastMouseXPosition, lastMouseYPosition, flags);
        }
    }
}

static void
pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
            uint32_t time, uint32_t axis, wl_fixed_t value)
{
}

static void
keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
               uint32_t format, int fd, uint32_t size)
{
}

static void
keyboard_handle_enter(void *data, struct wl_keyboard *keyboard,
              uint32_t serial, struct wl_surface *surface,
              struct wl_array *keys)
{
}

static void
keyboard_handle_leave(void *data, struct wl_keyboard *keyboard,
              uint32_t serial, struct wl_surface *surface)
{
}

static void
keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
            uint32_t serial, uint32_t time, uint32_t key,
            uint32_t state)
{
    vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
    vector<pxWindowNative*>::iterator i;
    unsigned long flags = 0;
    flags |= bShiftPressed ? PX_MOD_SHIFT:0;
    flags |= bCtrlPressed ? PX_MOD_CONTROL:0;
    flags |= bAltPressed ? PX_MOD_ALT:0;
    for (i = windowVector.begin(); i < windowVector.end(); i++)
    {
        pxWindowNative* w = (*i);
        if (state)
        {
            w->onKeyDown(keycodeFromNative(key),flags);
            w->onChar((char)(keycodeToAscii(keycodeFromNative(key),flags))); 
        }
        else
        {
            w->onKeyUp(keycodeFromNative(key),flags);
        }
    }
}

static void
keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard,
              uint32_t serial, uint32_t mods_depressed,
              uint32_t mods_latched, uint32_t mods_locked,
              uint32_t group)
{
    if (mods_depressed & MOD_SHIFT)
    {
        bShiftPressed = true;
    }
    else
    {
        bShiftPressed = false;
    }

    if (mods_depressed & MOD_ALT)
    {
        bAltPressed = true;
    }
    else
    {
        bAltPressed = false;
    }

    if (mods_depressed & MOD_CTRL)
    {
        bCtrlPressed = true;
    }
    else
    {
        bCtrlPressed = false;
    }
}

static void shell_surface_ping(void *data,
    struct wl_shell_surface *shell_surface, uint32_t serial)
{
    wl_shell_surface_pong(shell_surface, serial);
}

static void shell_surface_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height)
{
    pxWindowNative* w = (pxWindowNative*)data;

    if (w->getWaylandNative())
    {
        wl_egl_window_resize(w->getWaylandNative(), width, height, 0, 0);
    }
}

static void shellSurfaceConfigure(void *data,
    struct wl_shell_surface *shell_surface,
    uint32_t edges, int32_t width, int32_t height) { }

static void shellSurfacePing(void *data,
    struct wl_shell_surface *shell_surface, uint32_t serial)
{
    wl_shell_surface_pong(shell_surface, serial);
}

static const struct wl_shell_surface_listener shell_surface_listener =
{
    .ping = shellSurfacePing,
    .configure = shellSurfaceConfigure,
};

static void
buffer_release(void *data, struct wl_buffer *buffer)
{
    waylandBuffer *mybuf = (waylandBuffer *)data;

    mybuf->busy = 0;
}

static const struct wl_buffer_listener buffer_listener = {
    buffer_release
};

//end wayland callbacks

static void onWindowTimerFired( int sig, siginfo_t *si, void *uc )
{
    waylandDisplay* wDisplay = (waylandDisplay*)si->si_value.sival_ptr;
    vector<pxWindowNative*> windowVector = pxWindowNative::getNativeWindows();
    vector<pxWindowNative*>::iterator i;
    for (i = windowVector.begin(); i < windowVector.end(); i++)
    {
        pxWindowNative* w = (*i);
        w->animateAndRender();
    }
    wl_display_dispatch(wDisplay->display);
}

pxError displayRef::createWaylandDisplay()
{
    if (mDisplay == NULL)
    {
        mDisplay = new waylandDisplay();
    }

    //registry_listener
    mWaylandRegistryListener.global = registry_handle_global;
    mWaylandRegistryListener.global_remove = registry_handle_global_remove;

    //mWaylandPointerListener(mouse)
    mWaylandPointerListener.enter = pointer_handle_enter;
    mWaylandPointerListener.leave = pointer_handle_leave;
    mWaylandPointerListener.motion = pointer_handle_motion;
    mWaylandPointerListener.button = pointer_handle_button;
    mWaylandPointerListener.axis = pointer_handle_axis;

    mWaylandKeyboardListener.keymap = keyboard_handle_keymap;
    mWaylandKeyboardListener.enter = keyboard_handle_enter;
    mWaylandKeyboardListener.leave = keyboard_handle_leave;
    mWaylandKeyboardListener.key = keyboard_handle_key;
    mWaylandKeyboardListener.modifiers = keyboard_handle_modifiers;

    struct wl_registry *registry;

    mDisplay->display = wl_display_connect(NULL);
    if (mDisplay->display == NULL) {
        cout << "Error opening display" << endl;
        delete mDisplay;
        mDisplay = NULL;
        return PX_FAIL;
    }

    registry = wl_display_get_registry(mDisplay->display);
    wl_registry_add_listener(registry, &mWaylandRegistryListener,
        &mDisplay->compositor);
    wl_display_dispatch(mDisplay->display);
    wl_display_roundtrip(mDisplay->display);
    wl_registry_destroy(registry);
    return PX_OK;
}

void displayRef::cleanupWaylandDisplay()
{
    wl_pointer_destroy(mDisplay->pointer);
    wl_seat_destroy(mDisplay->seat);
    wl_shell_destroy(mDisplay->shell);
    wl_shm_destroy(mDisplay->shm);
    wl_compositor_destroy(mDisplay->compositor);
    wl_display_disconnect(mDisplay->display);
    if (mDisplay != NULL)
    {
        delete mDisplay;
    }
    mDisplay = NULL;
}

bool exitFlag = false;

pxWindowNative::~pxWindowNative()
{
    cleanupWaylandData();
}

pxError pxWindow::init(int left, int top, int width, int height)
{
    waylandDisplay* wDisplay = mDisplayRef.getDisplay();
    if (wDisplay == NULL)
    {
        cout << "Error initializing display\n" << endl;
        return PX_FAIL;
    }
    else
    {
        //mShellSurfaceListener
        mShellSurfaceListener.ping = shell_surface_ping;
        mShellSurfaceListener.configure = shell_surface_configure;

        mLastWidth = width;
        mLastHeight = height;
        mResizeFlag = true;
        mWaylandSurface = createWaylandSurface();

        wl_egl_window_resize(getWaylandNative(), width, height, 0, 0);

        registerWindow(this);
        this->onCreate();
    }
    return PX_OK;
}

pxError pxWindow::term()
{
    return PX_OK;
}

void pxWindow::invalidateRect(pxRect *r)
{
    invalidateRectInternal(r);
}

// This can be improved by collecting the dirty regions and painting
// when the event loop goes idle
void pxWindowNative::invalidateRectInternal(pxRect *r)
{
    //rendering for egl is now handled inside of onWindowTimerFired()
    //drawFrame();
}

bool pxWindow::visibility()
{
    return mVisible;
}

void pxWindow::setVisibility(bool visible)
{
    //todo - hide the window
    mVisible = visible;
}

pxError pxWindow::setAnimationFPS(uint32_t fps)
{
    mTimerFPS = fps;
    mLastAnimationTime = pxMilliseconds();
    return PX_OK;
}

void pxWindow::setTitle(const char* title)
{
    //todo
}

pxError pxWindow::beginNativeDrawing(pxSurfaceNative& s)
{
    //todo

    return PX_OK;
}

pxError pxWindow::endNativeDrawing(pxSurfaceNative& s)
{
    //todo

    return PX_OK;
}

// pxWindowNative

void pxWindowNative::onAnimationTimerInternal()
{
    if (mTimerFPS) onAnimationTimer();
}

int pxWindowNative::createAndStartEventLoopTimer(int timeoutInMilliseconds )
{
    struct sigevent         te;
    struct itimerspec       its;
    struct sigaction        sa;
    int                     sigNo = SIGRTMIN;
    
    if (mEventLoopTimerStarted)
    {
        stopAndDeleteEventLoopTimer();
    }
    
    displayRef dRef;
    waylandDisplay* wDisplay = dRef.getDisplay();

    //Set up signal handler
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = onWindowTimerFired;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sigNo, &sa, NULL) == -1)
    {
        fprintf(stderr, "Unable to setup signal handling for timer.\n");
        return(-1);
    }

    //Set and enable alarm
    te.sigev_notify = SIGEV_SIGNAL;
    te.sigev_signo = sigNo;
    te.sigev_value.sival_ptr = wDisplay;
    timer_create(CLOCK_REALTIME, &te, &mRenderTimerId);
    
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = timeoutInMilliseconds * 1000000;
    its.it_interval = its.it_value;
    timer_settime(mRenderTimerId, 0, &its, NULL);
    
    mEventLoopTimerStarted = true;

    return(0);
}

int pxWindowNative::stopAndDeleteEventLoopTimer()
{
    int returnValue = 0;
    if (mEventLoopTimerStarted)
    {
        returnValue = timer_delete(mRenderTimerId);
    }
    mEventLoopTimerStarted = false;
    return returnValue;
}

void pxWindowNative::runEventLoopOnce()
{
  vector<pxWindowNative*> windowVector = pxWindowNative::getNativeWindows();
  vector<pxWindowNative*>::iterator i;
  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->animateAndRender();
  }
  usleep(1000); //TODO - find out why pxSleepMS causes a crash on some devices
}



void pxWindowNative::runEventLoop()
{
    exitFlag = false;
    //createAndStartEventLoopTimer((int)mEventLoopInterval);
    displayRef dRef;
    waylandDisplay* display = dRef.getDisplay();
    vector<pxWindowNative*> windowVector = pxWindowNative::getNativeWindows();

    while(!exitFlag)
    {
        vector<pxWindowNative*>::iterator i;
        for (i = windowVector.begin(); i < windowVector.end(); i++)
        {
           pxWindowNative* w = (*i);
           w->animateAndRender();
        }
        wl_display_dispatch_pending(display->display);
        usleep(32*1000);
        //pxSleepMS(1000); // Breath
    }
}


void pxWindowNative::exitEventLoop()
{
    exitFlag = true;
}

struct wl_shell_surface* pxWindowNative::createWaylandSurface()
{
    struct wl_surface *surface;
    struct wl_shell_surface *shell_surface;
    waylandDisplay* display = mDisplayRef.getDisplay();

    initializeEgl();

    surface = wl_compositor_create_surface(display->compositor);

    if (surface == NULL)
        return NULL;

    shell_surface = wl_shell_get_shell_surface(display->shell, surface);

    if (shell_surface == NULL) {
        wl_surface_destroy(surface);
        return NULL;
    }

    wl_shell_surface_add_listener(shell_surface,
        &mShellSurfaceListener, this);
    wl_shell_surface_set_toplevel(shell_surface);
    wl_shell_surface_set_user_data(shell_surface, surface);
    wl_surface_set_user_data(surface, NULL);

    //egl stuff
    mEglNativeWindow = (struct wl_egl_window *)wl_egl_window_create(surface,
                         mLastWidth,
                         mLastHeight);

    mEglSurface =
        (EGLSurface)eglCreateWindowSurface(display->egl.dpy,
                       display->egl.conf,
                       (EGLNativeWindowType)mEglNativeWindow, NULL);

    EGLBoolean ret = eglMakeCurrent(display->egl.dpy, mEglSurface,
                 mEglSurface, display->egl.ctx);

    assert(ret == EGL_TRUE);

    eglSwapInterval(display->egl.dpy, 0);

    return shell_surface;
}

void pxWindowNative::cleanupWaylandData()
{
    waylandDisplay* display = mDisplayRef.getDisplay();

    //begin egl cleanup
    eglMakeCurrent(display->egl.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);

    eglDestroySurface(display->egl.dpy, mEglSurface);
    wl_egl_window_destroy(mEglNativeWindow);
    //end egl stuff

    if (mWaylandBuffer[0].buffer)
    {
        wl_buffer_destroy(mWaylandBuffer[0].buffer);
        mWaylandBuffer[0].buffer = NULL;
    }
    if (mWaylandBuffer[1].buffer)
    {
        wl_buffer_destroy(mWaylandBuffer[1].buffer);
        mWaylandBuffer[0].buffer = NULL;
    }
    struct wl_surface *surface = (struct wl_surface *)wl_shell_surface_get_user_data(mWaylandSurface);
    wl_shell_surface_destroy(mWaylandSurface);
    wl_surface_destroy(surface);
    mWaylandSurface = NULL;

    //more egl cleanup
    eglTerminate(display->egl.dpy);
    eglReleaseThread();
}

int pxWindowNative::set_cloexec_or_close(int fd)
{
long flags;

    if (fd == -1)
        return -1;

    flags = fcntl(fd, F_GETFD);
    if (flags == -1)
        goto err;

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
        goto err;

    return fd;

err:
    close(fd);
    return -1;
}

int pxWindowNative::create_tmpfile_cloexec(char *tmpname)
{
    int fd;

    fd = mkstemp(tmpname);
    if (fd >= 0) {
        fd = set_cloexec_or_close(fd);
        unlink(tmpname);
    }

    return fd;
}

int pxWindowNative::os_create_anonymous_file(off_t size)
{
    static const char templateFile[] = "/pxcore-shared-XXXXXX";
    const char *path;
    char *name;
    int fd;

    path = getenv("XDG_RUNTIME_DIR");
    if (!path)
    {
        return -1;
    }

    name = (char*)malloc(strlen(path) + sizeof(templateFile));
    if (!name)
        return -1;

    strcpy(name, path);
    strcat(name, templateFile);

    fd = create_tmpfile_cloexec(name);

    free(name);

    if (fd < 0)
        return -1;

    if (ftruncate(fd, size) < 0)
    {
        close(fd);
        return -1;
    }
    return fd;
}
int pxWindowNative::createShmBuffer(waylandDisplay *display, waylandBuffer *buffer,
          int width, int height, uint32_t format)
{
    struct wl_shm_pool *pool;
    int fd, size, stride;
    void *data;

    stride = width * 4;
    size = stride * height;

    fd = os_create_anonymous_file(size);
    if (fd < 0) {
        fprintf(stderr, "creating a buffer file for %d B failed: %m\n",
            size);
        return -1;
    }

    data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %m\n");
        close(fd);
        return -1;
    }

    pool = wl_shm_create_pool(display->shm, fd, size);
    buffer->buffer = wl_shm_pool_create_buffer(pool, 0,
                           width, height,
                           stride, format);
    wl_buffer_add_listener(buffer->buffer, &buffer_listener, buffer);
    wl_shm_pool_destroy(pool);
    close(fd);

    buffer->shm_data = data;

    return 0;
}

waylandBuffer* pxWindowNative::nextBuffer()
{
    waylandBuffer *buffer;
    int ret = 0;

    //use double buffering
    waylandBufferIndex = (waylandBufferIndex + 1) % 2;
    buffer = &mWaylandBuffer[waylandBufferIndex];

    if (!buffer->buffer)
    {
        ret = createShmBuffer(mDisplayRef.getDisplay(), buffer,
                    mLastWidth, mLastHeight,
                    WL_SHM_FORMAT_XRGB8888);

        if (ret < 0)
        {
            return NULL;
        }

        /* paint the padding */
        memset(buffer->shm_data, 0xff,
               mLastWidth * mLastHeight * 4);
    }

    return buffer;
}

void pxWindowNative::animateAndRender()
{
    static double lastAnimationTime = pxMilliseconds();
    double currentAnimationTime = pxMilliseconds();
    drawFrame(); 

    double animationDelta = currentAnimationTime-lastAnimationTime;
    if (mResizeFlag)
    {
        mResizeFlag = false;
        onSize(mLastWidth, mLastHeight);
        invalidateRectInternal(NULL);
    }

    if (mTimerFPS)
    {
        animationDelta = currentAnimationTime - getLastAnimationTime();

        if (animationDelta > (1000/mTimerFPS))
        {
            onAnimationTimerInternal();
            setLastAnimationTime(currentAnimationTime);
        }
    }
}

void pxWindowNative::setLastAnimationTime(double time)
{
    mLastAnimationTime = time;
}

double pxWindowNative::getLastAnimationTime()
{
    return mLastAnimationTime;
}

void pxWindowNative::drawFrame()
{
    displayRef dRef;

    waylandDisplay* wDisplay = dRef.getDisplay();
    struct wl_surface * waylandSurface = (struct wl_surface *)wl_shell_surface_get_user_data(mWaylandSurface);
    pxSurfaceNativeDesc d;
    d.windowWidth = mLastWidth;
    d.windowHeight = mLastHeight;
    waylandBuffer *buffer = nextBuffer();
    d.pixelData = (uint32_t*)buffer->shm_data;

    onDraw(&d);

    //this is needed only for egl
    if (WAYLAND_EGL_BUFFER_OPAQUE) {
    //if (true) {
        struct wl_region *region = wl_compositor_create_region(wDisplay->compositor);
        wl_region_add(region, 0, 0,
                  mLastWidth,
                  mLastHeight);
        wl_surface_set_opaque_region(waylandSurface, region);
        wl_region_destroy(region);
    } else {
        wl_surface_set_opaque_region(waylandSurface, NULL);
    }
    eglSwapBuffers(wDisplay->egl.dpy, mEglSurface);
}

//egl methods
void pxWindowNative::initializeEgl()
{
    static const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    const char *extensions;

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLint major, minor, n, count, i, size;
    EGLConfig *configs;
    EGLBoolean ret;

    if (WAYLAND_EGL_BUFFER_SIZE == 16)
        config_attribs[9] = 0;

    waylandDisplay* display = mDisplayRef.getDisplay();

    display->egl.dpy = eglGetDisplay(display->display);
    assert(display->egl.dpy);

    ret = eglInitialize(display->egl.dpy, &major, &minor);
    assert(ret == EGL_TRUE);
    ret = eglBindAPI(EGL_OPENGL_ES_API);
    assert(ret == EGL_TRUE);

    if (!eglGetConfigs(display->egl.dpy, NULL, 0, &count) || count < 1)
        assert(0);

    configs = (EGLConfig *)calloc(count, sizeof *configs);
    assert(configs);

    ret = eglChooseConfig(display->egl.dpy, config_attribs,
                  configs, count, &n);
    assert(ret && n >= 1);

    for (i = 0; i < n; i++) {
        eglGetConfigAttrib(display->egl.dpy,
                   configs[i], EGL_BUFFER_SIZE, &size);
        if (WAYLAND_EGL_BUFFER_SIZE == size) {
            display->egl.conf = configs[i];
            break;
        }
    }
    free(configs);
    if (display->egl.conf == NULL) {
        fprintf(stderr, "did not find config with buffer size %d\n", WAYLAND_EGL_BUFFER_SIZE);
        exit(EXIT_FAILURE);
    }

    display->egl.ctx = eglCreateContext(display->egl.dpy,
                        display->egl.conf,
                        EGL_NO_CONTEXT, context_attribs);
    assert(display->egl.ctx);

    display->swap_buffers_with_damage = NULL;
    extensions = eglQueryString(display->egl.dpy, EGL_EXTENSIONS);
    if (extensions &&
        strstr(extensions, "EGL_EXT_swap_buffers_with_damage") &&
        strstr(extensions, "EGL_EXT_buffer_age"))
        display->swap_buffers_with_damage =
            (PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC)
            eglGetProcAddress("eglSwapBuffersWithDamageEXT");

    if (display->swap_buffers_with_damage)
        printf("has EGL_EXT_buffer_age and EGL_EXT_swap_buffers_with_damage\n");
}

struct wl_egl_window* pxWindowNative::getWaylandNative()
{
    return mEglNativeWindow;
}

//end egl methods

void pxWindowNative::registerWindow(pxWindowNative* p)
{
    mWindowVector.push_back(p);
}

void pxWindowNative::unregisterWindow(pxWindowNative* p)
{
    vector<pxWindowNative*>::iterator i;

    for (i = mWindowVector.begin(); i < mWindowVector.end(); i++)
    {
        if ((*i) == p)
        {
            mWindowVector.erase(i);
            return;
        }
    }
}
