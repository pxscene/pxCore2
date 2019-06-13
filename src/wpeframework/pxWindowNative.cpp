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

// pxWindowNative.cpp

#include "../pxCore.h"
#include "../pxWindow.h"
#include "pxWindowNative.h"
#include "../pxTimer.h"
#include "../pxWindowUtil.h"
#include "../pxKeycodes.h"
#include "../rtLog.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h> //todo: remove when done with assert
#include <unistd.h> //for close()
#include <fcntl.h> //for files
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <vector>

#define WAYLAND_EGL_BUFFER_SIZE 32
#define WAYLAND_EGL_BUFFER_OPAQUE 0
#define WAYLAND_PX_CORE_FPS 30

std::vector<pxWindowNative*> pxWindowNative::mWindowVector;

bool exitFlag = false;

pxWindowNative::pxWindowNative(): mTimerFPS(0), mLastWidth(-1), mLastHeight(-1),
    mResizeFlag(false), mLastAnimationTime(0.0), mVisible(false), mDirty(true),
    mEglNativeWindow(NULL), mEGLSurface(NULL)
{
}

pxWindowNative::~pxWindowNative()
{
    cleanup();
}

pxError pxWindow::init(int left, int top, int width, int height)
{
    mLastWidth = width;
    mLastHeight = height;
    mResizeFlag = true;
    createWindowSurface();

    registerWindow(this);
    this->onCreate();
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
  mDirty = true;
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

void pxWindowNative::runEventLoopOnce()
{
  std::vector<pxWindowNative*> windowVector = pxWindowNative::getNativeWindows();
  std::vector<pxWindowNative*>::iterator i;
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
    std::vector<pxWindowNative*> windowVector = pxWindowNative::getNativeWindows();

    int framerate = WAYLAND_PX_CORE_FPS;

    char const *s = getenv("PXCORE_FRAMERATE");
    if (s)
    {
      int fps = atoi(s);
      if (fps > 0)
      {
        framerate = fps;
      }
    }

    rtLogInfo("pxcore framerate: %d", framerate);

    double maxSleepTime = (1000 / framerate) * 1000;
    rtLogInfo("max sleep time in microseconds: %f", maxSleepTime);
    while(!exitFlag)
    {
        double startMicroseconds = pxMicroseconds();
        std::vector<pxWindowNative*>::iterator i;
        for (i = windowVector.begin(); i < windowVector.end(); i++)
        {
           pxWindowNative* w = (*i);
           w->animateAndRender();
        }
        double processTime = (int)pxMicroseconds() - (int)startMicroseconds;
        if (processTime < 0)
        {
          processTime = 0;
        }
        if (processTime < maxSleepTime)
        {
          int sleepTime = (int)maxSleepTime-(int)processTime;
          usleep(sleepTime);
        }
    }
}

void pxWindowNative::exitEventLoop()
{
    exitFlag = true;
}

bool pxWindowNative::createWindowSurface()
{
    initializeEgl();

    mEglNativeWindow = wpeDisplay->GetGraphicsSurface()->Native();

    mEGLSurface =
        (EGLSurface)eglCreateWindowSurface(mEGLDisplay,
                       mEGLConfig,
                       mEglNativeWindow, NULL);

    EGLBoolean ret = eglMakeCurrent(mEGLDisplay, mEGLSurface,
                 mEGLSurface, mEGLContext);

    assert(ret == EGL_TRUE);

    eglSwapInterval(mEGLDisplay, 0);
    eglSurfaceAttrib(mEGLDisplay, mEGLSurface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);

    return true;
}

void pxWindowNative::cleanup()
{
    //begin egl cleanup
    eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);

    eglDestroySurface(mEGLDisplay, mEGLSurface);
    mEglNativeWindow = NULL;
    //end egl stuff

    mEGLSurface = NULL;

    //more egl cleanup
    eglTerminate(mEGLDisplay);
    wpeDisplay->Release();
    wpeDisplay = nullptr;

    eglReleaseThread();
}

void pxWindowNative::animateAndRender()
{
    drawFrame();

    if (mResizeFlag)
    {
        mResizeFlag = false;
        onSize(mLastWidth, mLastHeight);
        invalidateRectInternal(NULL);
    }

    onAnimationTimerInternal();
}

void pxWindowNative::drawFrame()
{
    if (!mDirty)
    {
      return;
    }

    pxSurfaceNativeDesc d;
    d.windowWidth = mLastWidth;
    d.windowHeight = mLastHeight;

    onDraw(&d);
    eglSwapBuffers(mEGLDisplay, mEGLSurface);
    mDirty = false;
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

    wpeDisplay = WPEFramework::Display::Instance();
    EGLNativeWindowType *nativewindow =
    static_cast<EGLNativeWindowType*>(wpeDisplay->GetGraphicsSurface()->Native());

    mEGLDisplay = eglGetDisplay(wpeDisplay->Native());
    assert(mEGLDisplay);

    ret = eglInitialize(mEGLDisplay, &major, &minor);
    assert(ret == EGL_TRUE);
    ret = eglBindAPI(EGL_OPENGL_ES_API);
    assert(ret == EGL_TRUE);

    if (!eglGetConfigs(mEGLDisplay, NULL, 0, &count) || count < 1)
        assert(0);

    configs = (EGLConfig *)calloc(count, sizeof *configs);
    assert(configs);

    ret = eglChooseConfig(mEGLDisplay, config_attribs,
                  configs, count, &n);
    assert(ret && n >= 1);

    for (i = 0; i < n; i++) {
        eglGetConfigAttrib(mEGLDisplay,
                   configs[i], EGL_BUFFER_SIZE, &size);
        if (WAYLAND_EGL_BUFFER_SIZE == size) {
            mEGLConfig = configs[i];
            break;
        }
    }
    free(configs);
    if (mEGLConfig == NULL) {
        fprintf(stderr, "did not find config with buffer size %d\n", WAYLAND_EGL_BUFFER_SIZE);
        exit(EXIT_FAILURE);
    }

    mEGLContext = eglCreateContext(mEGLDisplay,
                        mEGLConfig,
                        EGL_NO_CONTEXT, context_attribs);
    assert(mEGLContext);

    mSwapBuffersWithDamage = NULL;
    extensions = eglQueryString(mEGLDisplay, EGL_EXTENSIONS);
    if (extensions &&
        strstr(extensions, "EGL_EXT_swap_buffers_with_damage") &&
        strstr(extensions, "EGL_EXT_buffer_age"))
        mSwapBuffersWithDamage =
            (PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC)
            eglGetProcAddress("eglSwapBuffersWithDamageEXT");

    if (mSwapBuffersWithDamage)
        printf("has EGL_EXT_buffer_age and EGL_EXT_swap_buffers_with_damage\n");
}

//end egl methods

void pxWindowNative::registerWindow(pxWindowNative* p)
{
    mWindowVector.push_back(p);
}

void pxWindowNative::unregisterWindow(pxWindowNative* p)
{
    std::vector<pxWindowNative*>::iterator i;

    for (i = mWindowVector.begin(); i < mWindowVector.end(); i++)
    {
        if ((*i) == p)
        {
            mWindowVector.erase(i);
            return;
        }
    }
}
