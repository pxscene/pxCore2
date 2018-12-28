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
#include <unistd.h>
#include <signal.h>
#include <vector>

#define ESSOS_PX_CORE_FPS 30

#define MOD_SHIFT	0x01
#define MOD_ALT		0x08
#define MOD_CTRL	0x04

bool bShiftLeftPressed = false;
bool bShiftRightPressed = false;
bool bAltLeftPressed = false;
bool bAltRightPressed = false;
bool bCtrlLeftPressed = false;
bool bCtrlRightPressed = false;

static int gDisplayWidth = 0;
static int gDisplayHeight = 0;

essosDisplay* displayRef::mDisplay  = NULL;
int displayRef::mRefCount = 0;
std::vector<pxWindowNative*> pxWindowNative::mWindowVector;
bool pxWindowNative::mEventLoopTimerStarted = false;
float pxWindowNative::mEventLoopInterval = 1000.0 / (float)ESSOS_PX_CORE_FPS;
timer_t pxWindowNative::mRenderTimerId;

static void terminated( void * )
{
}

static EssTerminateListener terminateListener=
    {
        terminated
    };

static void keyPressed( void */*userData*/, unsigned int key )
{
    switch( key )
    {
        case PX_KEY_NATIVE_SHIFT:
            bShiftRightPressed = true;
        break;
        case PX_KEY_NATIVE_SHIFT_LEFT:
            bShiftLeftPressed = true;
        break;
        case PX_KEY_NATIVE_CONTROL:
            bCtrlRightPressed = true;
        break;
        case PX_KEY_NATIVE_CONTROL_LEFT:
            bCtrlLeftPressed = true;
        break;
        case PX_KEY_NATIVE_ALT:
            bAltRightPressed = true;
        break;
        case PX_KEY_NATIVE_ALT_LEFT:
            bAltLeftPressed = true;
        break;
        default:
            break;
    }

    std::vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
    std::vector<pxWindowNative*>::iterator i;
    unsigned long flags = 0;
    flags |= (bShiftRightPressed || bShiftLeftPressed) ? PX_MOD_SHIFT:0;
    flags |= (bCtrlRightPressed || bCtrlLeftPressed ) ? PX_MOD_CONTROL:0;
    flags |= (bAltRightPressed || bAltLeftPressed) ? PX_MOD_ALT:0;
    for (i = windowVector.begin(); i < windowVector.end(); i++)
    {
        pxWindowNative* w = (*i);
        w->onKeyDown(keycodeFromNative(key),flags);
        w->onChar(keycodeToAscii(keycodeFromNative(key), flags));
    }
}

static void keyReleased( void */*userData*/, unsigned int key )
{
    switch( key )
    {
        case PX_KEY_NATIVE_SHIFT:
            bShiftRightPressed = false;
        break;
        case PX_KEY_NATIVE_SHIFT_LEFT:
            bShiftLeftPressed = false;
        break;
        case PX_KEY_NATIVE_CONTROL:
            bCtrlRightPressed = false;
        break;
        case PX_KEY_NATIVE_CONTROL_LEFT:
            bCtrlLeftPressed = false;
        break;
        case PX_KEY_NATIVE_ALT:
            bAltRightPressed = false;
        break;
        case PX_KEY_NATIVE_ALT_LEFT:
            bAltLeftPressed = false;
        break;
        default:
            break;
    }

    std::vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
    std::vector<pxWindowNative*>::iterator i;
    unsigned long flags = 0;
    flags |= (bShiftRightPressed || bShiftLeftPressed) ? PX_MOD_SHIFT:0;
    flags |= (bCtrlRightPressed || bCtrlLeftPressed ) ? PX_MOD_CONTROL:0;
    flags |= (bAltRightPressed || bAltLeftPressed) ? PX_MOD_ALT:0;
    for (i = windowVector.begin(); i < windowVector.end(); i++)
    {
        pxWindowNative* w = (*i);
        w->onKeyUp(keycodeFromNative(key),flags);
    }
}

static EssKeyListener keyListener=
    {
        keyPressed,
        keyReleased
    };

#ifdef ESSOS_SETTINGS_AND_TOUCH_SUPPORT

static void displaySize( void */*userData*/, int width, int height )
{
  std::vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  std::vector<pxWindowNative*>::iterator i;
  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onSizeUpdated(width,height);
  }
}

static EssSettingsListener settingsListener =
   {
     displaySize
   };

static void touchDown( void */*userData*/, int id, int x, int y )
{
  std::vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  std::vector<pxWindowNative*>::iterator i;
  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onTouchDown(id, x, y);
  }
}

static void touchUp( void */*userData*/, int id )
{
  std::vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  std::vector<pxWindowNative*>::iterator i;
  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onTouchUp(id);
  }
}

static void touchMotion( void */*userData*/, int id, int x, int y )
{
  std::vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  std::vector<pxWindowNative*>::iterator i;
  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onTouchMotion(id, x, y);
  }
}

static void touchFrame( void */*userData*/ )
{
  std::vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  std::vector<pxWindowNative*>::iterator i;
  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onTouchFrame();
  }
}

static EssTouchListener touchListener=
{
   touchDown,
   touchUp,
   touchMotion,
   touchFrame
};

#endif //ESSOS_SETTINGS_AND_TOUCH_SUPPORT

static void pointerMotion( void */*userData*/, int x, int y )
{
    std::vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
    std::vector<pxWindowNative*>::iterator i;
    for (i = windowVector.begin(); i < windowVector.end(); i++)
    {
        pxWindowNative* w = (*i);
        w->onMouseMove(x,y);
    }
}

static void pointerButtonPressed( void */*userData*/, int button, int x, int y )
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

    std::vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
    std::vector<pxWindowNative*>::iterator i;
    for (i = windowVector.begin(); i < windowVector.end(); i++)
    {
        pxWindowNative* w = (*i);
        w->onMouseDown(x, y, flags);
    }
}

static void pointerButtonReleased( void */*userData*/, int button, int x, int y )
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

    std::vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
    std::vector<pxWindowNative*>::iterator i;
    for (i = windowVector.begin(); i < windowVector.end(); i++)
    {
        pxWindowNative* w = (*i);
        w->onMouseUp(x, y, flags);
    }
}

static EssPointerListener pointerListener=
    {
        pointerMotion,
        pointerButtonPressed,
        pointerButtonReleased
    };


static void onWindowTimerFired( int sig, siginfo_t *si, void *uc )
{
  std::vector<pxWindowNative*> windowVector = pxWindowNative::getNativeWindows();
  std::vector<pxWindowNative*>::iterator i;
  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->animateAndRender();
  }
}

displayRef::displayRef()
{
    if (mRefCount == 0)
    {
        mRefCount++;
        createEssosDisplay();
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
        cleanupEssosDisplay();
    }
}

essosDisplay* displayRef::getDisplay() const
{
    return mDisplay;
}

pxError displayRef::createEssosDisplay()
{
    if (mDisplay == NULL)
    {
        mDisplay = new essosDisplay();
    }

    return PX_OK;
}

void displayRef::cleanupEssosDisplay()
{

    if (mDisplay != NULL)
    {
        delete mDisplay;
    }
    mDisplay = NULL;
}

bool exitFlag = false;

pxWindowNative::pxWindowNative(): mTimerFPS(0), mLastWidth(-1), mLastHeight(-1),
    mResizeFlag(false), mLastAnimationTime(0.0), mVisible(false), mDirty(true)
{
}

pxWindowNative::~pxWindowNative()
{
    cleanupEssos();
}

pxError pxWindow::init(int left, int top, int width, int height)
{
    essosDisplay* d = mDisplayRef.getDisplay();
    if (d == NULL)
    {
        std::cout << "Error initializing display\n" << std::endl;
        return PX_FAIL;
    }
    else
    {
        bool useWayland = false;
        char const *s = getenv("PXCORE_ESSOS_WAYLAND");
        if (s)
        {
            int value = atoi(s);
            if (value > 0)
            {
                useWayland = true;
            }
        }

        int keyInitialDelay = 500;
        char const *keyDelay = getenv("SPARK_KEY_INITIAL_DELAY");
        if (keyDelay)
        {
            int value = atoi(keyDelay);
            if (value > 0)
            {
                keyInitialDelay = value;
            }
        }

        int keyRepeatInterval = 250;
        char const *repeatInterval = getenv("SPARK_KEY_REPEAT_INTERVAL");
        if (repeatInterval)
        {
            int value = atoi(repeatInterval);
            if (value > 0)
            {
                keyRepeatInterval = value;
            }
        }


        mLastWidth = width;
        mLastHeight = height;
        mResizeFlag = true;

        registerWindow(this);

        bool error = false;
        rtLogInfo("using wayland: %s\n", useWayland ? "true" : "false");
        rtLogInfo("initial key delay: %d repeat interval: %d", keyInitialDelay, keyRepeatInterval);
        d->ctx = EssContextCreate();

        if (d->ctx)
        {
            if ( !EssContextSetUseWayland( d->ctx, useWayland ) )
            {
                error = true;
            }
            if ( !EssContextSetTerminateListener( d->ctx, 0, &terminateListener ) )
            {
                error = true;
            }

            if ( !EssContextSetKeyListener( d->ctx, 0, &keyListener ) )
            {
                error = true;
            }

            if ( !EssContextSetPointerListener( d->ctx, 0, &pointerListener ) )
            {
                error = true;
            }
#ifdef ESSOS_SETTINGS_AND_TOUCH_SUPPORT
            if ( !EssContextSetTouchListener( d->ctx, 0, &touchListener ) )
            {
                error= true;
            }
#endif //ESSOS_SETTINGS_AND_TOUCH_SUPPORT
            if ( !EssContextSetKeyRepeatInitialDelay(d->ctx, keyInitialDelay))
            {
                error = true;
            }
            if ( !EssContextSetKeyRepeatPeriod(d->ctx, keyRepeatInterval))
            {
                error = true;
            }
#ifdef ESSOS_SETTINGS_AND_TOUCH_SUPPORT
            if (!EssContextSetSettingsListener(d->ctx, 0, &settingsListener))
            {
                error = true;
            }
#endif //ESSOS_SETTINGS_AND_TOUCH_SUPPORT
            if ( !error )
            {
                if ( !EssContextStart( d->ctx ) )
                {
                    error = true;
                }
                else if ( !EssContextGetDisplaySize( d->ctx, &gDisplayWidth, &gDisplayHeight ) )
                {
                    error= true;
                }
            }
            if ( error )
            {
                const char *detail= EssContextGetLastErrorDetail( d->ctx );
                rtLogError("Essos error: (%s)\n", detail );
            }
        }

        eglSurfaceAttrib(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);

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
    essosDisplay* eDisplay = dRef.getDisplay();

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
    te.sigev_value.sival_ptr = eDisplay;
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
    displayRef dRef;
    std::vector<pxWindowNative*> windowVector = pxWindowNative::getNativeWindows();

    int framerate = ESSOS_PX_CORE_FPS;

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

void pxWindowNative::cleanupEssos()
{
    essosDisplay* eDisplay = mDisplayRef.getDisplay();

    EssContextDestroy( eDisplay->ctx );
    eDisplay->ctx = NULL;
}

void pxWindowNative::onSizeUpdated(int width, int height)
{
  mLastWidth = width;
  mLastHeight = height;
  mResizeFlag = true;
  onSize(width, height);
}


void pxWindowNative::onTouchDown(int id, int x, int y)
{
  (void)id;
  (void)x;
  (void)y;
  //TODO
}

void pxWindowNative::onTouchUp(int id)
{
  (void)id;
  //TODO
}

void pxWindowNative::onTouchMotion(int id, int x, int y)
{
  (void)id;
  (void)x;
  (void)y;
  //TODO
}

void pxWindowNative::onTouchFrame()
{
  //TODO
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

    essosDisplay* eDisplay = dRef.getDisplay();
 
    if (!mDirty)
    {
      EssContextRunEventLoopOnce( eDisplay->ctx );
      return;
    }

    if (eDisplay->ctx != NULL)
    {
      EssContextRunEventLoopOnce( eDisplay->ctx );
      pxSurfaceNativeDesc d;
      d.windowWidth = mLastWidth;
      d.windowHeight = mLastHeight;
      onDraw(&d);
      EssContextUpdateDisplay( eDisplay->ctx );
    }

    mDirty = false;
}

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
