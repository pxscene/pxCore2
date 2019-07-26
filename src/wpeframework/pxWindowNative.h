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

#ifndef PX_WINDOW_NATIVE_H
#define PX_WINDOW_NATIVE_H

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
#include "pxWPECompositor.h"

#ifndef EGL_EXT_swap_buffers_with_damage
#define EGL_EXT_swap_buffers_with_damage 1
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC)(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects);
#endif

#ifndef EGL_EXT_buffer_age
#define EGL_EXT_buffer_age 1
#define EGL_BUFFER_AGE_EXT			0x313D
#endif

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
    void drawFrame();

    bool createWindowSurface();

    int mTimerFPS;
    int mLastWidth, mLastHeight;
    bool mResizeFlag;
    double mLastAnimationTime;
    bool mVisible;
    bool mDirty;

    //egl content
    void initializeEgl();
    EGLNativeWindowType *mEglNativeWindow;
    //end egl content

    static void registerWindow(pxWindowNative* p);
    static void unregisterWindow(pxWindowNative* p); //call this method somewhere
    static std::vector<pxWindowNative*> mWindowVector;

private:
   void cleanup();

    WPEFramework::Display* wpeDisplay;

    // generic egl stuff
    EGLDisplay mEGLDisplay;
    EGLSurface mEGLSurface;
    EGLContext mEGLContext;
    EGLConfig mEGLConfig;

    PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC mSwapBuffersWithDamage;
};

#endif
