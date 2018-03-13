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

// pxWindowNativeDfb.cpp

#include "../pxCore.h"
#include "../pxWindow.h"
#include "../pxTimer.h"
#include "../pxWindowUtil.h"
#include "../pxKeycodes.h"

#include "pxWindowNativeDfb.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

using namespace std;

#define DFB_PX_CORE_FPS 30

#define USE_DFB_FLIPPING

#define USE_DRAW_THREAD
//#define USE_DRAW_ALARM   /// BAD >> Results in " Direct/Thread: Started 'SigHandler' " error when run with Node


IDirectFB              *dfb        = NULL;
IDirectFBSurface       *dfbSurface = NULL;
IDirectFBEventBuffer   *dfbBuffer  = NULL;

DFBSurfaceDescription   dfbDescription;

#ifdef USE_DRAW_THREAD
void *draw_func(void *ptr);
static pthread_t draw_thread = 0;
#endif

// TODO ... probably better per window.
static int cursor_x = 0;
static int cursor_y = 0;

// DSPF_ABGR == Ubuntu
//
// DSPF_ARGB == RNG150

DFBSurfacePixelFormat  dfbPixelformat = DSPF_UNKNOWN;

bool needsFlip = true;
bool exitFlag  = false;

#define DFB_CHECK(x...)                                   \
{                                                         \
  DFBResult err = x;                                      \
                                                          \
  if (err != DFB_OK)                                      \
  {                                                       \
    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );\
    DirectFBErrorFatal( #x, err );                        \
  }                                                       \
}

#define DFB_ERROR(err)                                    \
  fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__,     \
  DirectFBErrorString(err));


dfbDisplay* displayRef::mDisplay  = NULL;
int         displayRef::mRefCount = 0;

vector<pxWindowNative*> pxWindowNative::mWindowVector;

bool    pxWindowNative::mEventLoopTimerStarted = false;
float   pxWindowNative::mEventLoopInterval = 1000.0 / (float)DFB_PX_CORE_FPS;
timer_t pxWindowNative::mRenderTimerId;


char* p2str(DFBSurfacePixelFormat fmt);

void onKeyboardDown(int32_t key, uint32_t mods);
void onKeyboardUp(int32_t key, uint32_t mods);
void onKeyboard(int32_t key, uint32_t mods, bool down);
void onTimer(uint32_t v);
void onMouseMotion(int32_t x, int32_t y);
void onMouse(uint32_t button, int32_t state, int32_t x, int32_t y);


//#define EPRINTF(...) printf(__VA_ARGS__)
#define EPRINTF(f_, ...)

static void
ProcessInputEvent(DFBInputEvent *ievt)
{
  uint32_t flags = 0;

  if( ievt == NULL )   return;

  if( ievt->modifiers == DIMM_SHIFT )   flags |= PX_MOD_SHIFT;
  if( ievt->modifiers == DIMM_CONTROL ) flags |= PX_MOD_CONTROL;
  if( ievt->modifiers == DIMM_META )    flags |= PX_MOD_ALT;

  switch (ievt->type)
  {
    case DIET_AXISMOTION:
      // EPRINTF("\n DIET_AXISMOTION");
      if (ievt->flags & DIEF_AXISABS)
      {
        if (ievt->axis == DIAI_X)
        {
          cursor_x = ievt->axisabs;
        }
        else if (ievt->axis == DIAI_Y)
        {
          cursor_y = ievt->axisabs;
        }
      }
      if (ievt->flags & DIEF_AXISREL)
      {
        if (ievt->axis == DIAI_X)
        {
          cursor_x += ievt->axisrel;
        }
        else if (ievt->axis == DIAI_Y)
        {
          cursor_y += ievt->axisrel;
        }
      }

      //printf("MOUSE >> (%d,%d)\n", cursor_x, cursor_y);

      onMouseMotion(cursor_x, cursor_y);
      break;

    case DIET_KEYPRESS:
      EPRINTF("\n DIET_KEYPRESS");

      onKeyboardDown(ievt->key_id, flags);
      break;

    case DIET_KEYRELEASE:
      EPRINTF("\n DIET_KEYRELEASE");

      onKeyboardUp(ievt->key_id, flags);
      break;

    case DIET_BUTTONPRESS:
      EPRINTF("\n DIET_BUTTONPRESS");

      onMouse(ievt->buttons, DIET_BUTTONPRESS,  cursor_x, cursor_y);
      break;

    case DIET_BUTTONRELEASE:
      EPRINTF("\n DIET_BUTTONRELEASE");

      onMouse(ievt->buttons, DIET_BUTTONRELEASE,  cursor_x, cursor_y);
      break;

    default:
      break;              /* please gcc */
  }
}


//start dfb callbacks

void eventLoop()
{
  printf("************************* %s - STARTED\n",__PRETTY_FUNCTION__);// JUNK

  while (!exitFlag)
  {
    DFB_CHECK (dfbBuffer->WaitForEvent(dfbBuffer)); // Block waiting for next event...

    DFBInputEvent  iEvent;

    // Get any INPUT events in case we need them
    while (dfbBuffer->GetEvent(dfbBuffer, DFB_EVENT(&iEvent)) == DFB_OK)
    {
      ProcessInputEvent( &iEvent);
    }
  }//WHILE

  printf("************************* %s ... exiting...\n",__PRETTY_FUNCTION__);// JUNK
}

void display()
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->animateAndRender();
  }

#ifdef USE_DFB_FLIPPING
  if(dfbSurface && needsFlip)
  {
    dfbSurface->Flip( dfbSurface, NULL, (DFBSurfaceFlipFlags) DSFLIP_WAITFORSYNC);

    needsFlip = false;
  }
#endif

}

void onTimer(uint32_t /*v*/)
{
  display();
}

void onMouse(uint32_t button, int32_t state, int32_t x, int32_t y)
{
  uint32_t flags = 0;

  // Allow for multiple buttons
  if( (button & DIBM_LEFT) &&
      (button & DIBM_RIGHT) )
  {
    flags = PX_LEFTBUTTON | PX_RIGHTBUTTON;
  }
  else
  if( (button & DIBM_LEFT)   &&
      (button & DIBM_MIDDLE) &&
      (button & DIBM_RIGHT) )
  {
    flags = PX_LEFTBUTTON | PX_MIDDLEBUTTON | PX_RIGHTBUTTON;
  }
  else
  if(button & DIBM_LEFT)
  {
    flags = PX_LEFTBUTTON;
  }
  else
  if(button & DIBM_MIDDLE)
  {
    flags = PX_MIDDLEBUTTON;
  }
  else
  if(button & DIBM_RIGHT)
  {
    flags = PX_RIGHTBUTTON;
  }

  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);

    if (state == DIET_BUTTONPRESS)
    {
      w->onMouseDown(x, y, flags);
    }
    else
    if(state == DIET_BUTTONRELEASE)
    {
      w->onMouseUp(x, y, flags);
    }
    else
    {
      w->onMouseMove(x, y);
    }
  }//FOR
}

void onMouseMotion(int32_t x, int32_t y)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onMouseMove(x, y);
  }
}

void onMousePassiveMotion(int32_t x, int32_t y)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onMouseMove(x, y);
  }
}

void onKeyboardDown(int32_t key, uint32_t flags)
{
  onKeyboard(key, flags, true);
}

void onKeyboardUp(int32_t key, uint32_t flags)
{
  onKeyboard(key, flags, false);
}

void onKeyboard(int32_t key, uint32_t flags, bool down)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);

    if(down)
    {
      w->onKeyDown(keycodeFromNative((int)key), flags);
    }
    else
    {
       w->onKeyUp(keycodeFromNative((int)key), flags);
    }
  }
}

void onKeyboardSpecial(int key, int x, int y)
{
   (void) x; (void) y;

  int keycode = key;
  switch (key)
  {
    case DIKI_F1:
      keycode = PX_KEY_NATIVE_F1;
      break;
    case DIKI_F2:
      keycode = PX_KEY_NATIVE_F2;
      break;
    case DIKI_F3:
      keycode = PX_KEY_NATIVE_F3;
      break;
    case DIKI_F4:
      keycode = PX_KEY_NATIVE_F4;
      break;
    case DIKI_F5:
      keycode = PX_KEY_NATIVE_F5;
      break;
    case DIKI_F6:
      keycode = PX_KEY_NATIVE_F6;
      break;
    case DIKI_F7:
      keycode = PX_KEY_NATIVE_F7;
      break;
    case DIKI_F8:
      keycode = PX_KEY_NATIVE_F8;
      break;
    case DIKI_F9:
      keycode = PX_KEY_NATIVE_F9;
      break;
    case DIKI_F10:
      keycode = PX_KEY_NATIVE_F10;
      break;
    case DIKI_F11:
      keycode = PX_KEY_NATIVE_F11;
      break;
    case DIKI_F12:
      keycode = PX_KEY_NATIVE_F12;
      break;
    case DIKI_LEFT:
      keycode = PX_KEY_NATIVE_LEFT;
      break;
    case DIKI_UP:
      keycode = PX_KEY_NATIVE_UP;
      break;
    case DIKI_RIGHT:
      keycode = PX_KEY_NATIVE_RIGHT;
      break;
    case DIKI_DOWN:
      keycode = PX_KEY_NATIVE_DOWN;
      break;
    case DIKI_PAGE_UP:
      keycode = PX_KEY_NATIVE_PAGEUP;
      break;
    case DIKI_PAGE_DOWN:
      keycode = PX_KEY_NATIVE_PAGEDOWN;
      break;
    case DIKI_HOME:
      keycode = PX_KEY_NATIVE_HOME;
      break;
    case DIKI_END:
      keycode = PX_KEY_NATIVE_END;
      break;
    case DIKI_INSERT:
      keycode = PX_KEY_NATIVE_INSERT;
      break;
    default:
      printf("unknown special key: %d\n",key);
  }
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    // JR Did I mention Glut keyboard support is not very good
    w->onKeyDown(keycodeFromNative((int)keycode), 0);
    w->onKeyUp(keycodeFromNative((int)keycode), 0);
  }
}

//end DFB callbacks

void onEntry(int32_t state)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    if (state == DWET_LEAVE)
      w->onMouseLeave();
  }

}

displayRef::displayRef()
{
  if (mRefCount == 0)
  {
    mRefCount++;
    createDfbDisplay();
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
    cleanupDfbDisplay();
  }
}

dfbDisplay* displayRef::getDisplay() const
{
  return mDisplay;
}

void displayRef::cleanupDfbDisplay()
{
  if (mDisplay != NULL)
  {
    delete mDisplay;
  }
  mDisplay = NULL;
}

pxWindowNative::~pxWindowNative()
{
  cleanupDfbWindow();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void pxWindowNative::createDfbWindow(int left, int top, int width, int height)
{
  (void) left;  (void) top;

  dfbDescription.flags = DFBSurfaceDescriptionFlags(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
  dfbDescription.caps  = DFBSurfaceCapabilities(DSCAPS_PRIMARY);//  | DSCAPS_DOUBLE );// | DSCAPS_FLIPPING);

  int SW = 0, SH = 0;

  dfbDescription.width  = width;
  dfbDescription.height = height;

  DFB_CHECK( dfb->CreateSurface( dfb, &dfbDescription, &dfbSurface ) );
  DFB_CHECK( dfbSurface->GetSize( dfbSurface, &SW, &SH )             );

  DFB_CHECK(dfbSurface->GetPixelFormat( dfbSurface, &dfbPixelformat ));

  printf("\n\n NOTE: WxH >> %d x %d ... pixelformat: %s ******* \n\n", SW, SH, p2str(dfbPixelformat) );

  // Clear initially
  DFB_CHECK( dfbSurface->Clear( dfbSurface, 0, 0, 0,  0 ) );

  // Enable Alpha
  DFB_CHECK (dfbSurface->SetBlittingFlags(dfbSurface, DSBLIT_BLEND_ALPHACHANNEL));
}


pxError displayRef::createDfbDisplay()
{
  if (mDisplay == NULL)
  {
    mDisplay = new dfbDisplay();
  }

  DirectFBInit( NULL, NULL );

  DirectFBSetOption ("quiet",              NULL); // Suppress banner
//  DirectFBSetOption ("hardware",           NULL); // Enable hardware acceleration
//  DirectFBSetOption ("software",           NULL); // Enable software fallback
//  DirectFBSetOption ("bg-none",            NULL);
//  DirectFBSetOption ("dma",                NULL);
//  DirectFBSetOption ("mmx",                NULL);
//  DirectFBSetOption ("gfxcard-stats",      NULL);
//  DirectFBSetOption ("forcepremultiplied", NULL);

#ifndef USE_DFB_FLIPPING
//  DirectFBSetOption ("autoflip-window",   "true");
#endif

  DirectFBCreate( &dfb );
  DFB_CHECK(dfb->SetCooperativeLevel (dfb, DFSCL_NORMAL));

  if(dfb == NULL)
  {
    printf("\nERROR:  %s failed.  DFB == NULL\n", __PRETTY_FUNCTION__);
    exit(-1);
  }

  // Create an event buffer with key capable devices attached.
  DFB_CHECK( dfb->CreateInputEventBuffer( dfb, DICAPS_ALL, DFB_FALSE, &dfbBuffer ) );

  return PX_OK;
}

void pxWindowNative::cleanupDfbWindow()
{
  if(dfbBuffer)
  {
    dfbBuffer->Release(dfbBuffer);
    dfbBuffer = NULL;
  }

  if(dfbSurface)
  {
    dfbSurface->Release(dfbSurface);
    dfbSurface = NULL;
  }
}


pxError pxWindow::init(int left, int top, int width, int height)
{
  dfbDisplay* display = mDisplayRef.getDisplay();

  if (display == NULL)
  {
    printf("#### %s   (%d,%d) %d x %d  <<<<<<<<<<< ERROR \n", __PRETTY_FUNCTION__, left, top, width, height);

    cout << "Error initializing display\n" << endl;
    return PX_FAIL;
  }
  else
  {
    mLastWidth  = width;
    mLastHeight = height;
    mResizeFlag = true;

    createDfbWindow(left,top,width,height);

    // XXX: Need to register callbacks after window is created
    // TODO   glutReshapeFunc(reshape);

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
void pxWindowNative::invalidateRectInternal(pxRect * /*r*/)
{
  drawFrame();
}

bool pxWindow::visibility()
{
  return mVisible;
}

void pxWindow::setVisibility(bool visible)
{
  visible = visible;
}

pxError pxWindow::setAnimationFPS(uint32_t fps)
{
  mTimerFPS = (int) fps;
  mLastAnimationTime = pxMilliseconds();

  return PX_OK;
}

void pxWindow::setTitle(const char* title)
{
  title = title;
  //glutSetWindowTitle(title);
}

pxError pxWindow::beginNativeDrawing(pxSurfaceNative& /*s*/)
{
  //TODO

  return PX_OK;
}

pxError pxWindow::endNativeDrawing(pxSurfaceNative& /*s*/)
{
  //TODO

  return PX_OK;
}

// pxWindowNative

void pxWindowNative::onAnimationTimerInternal()
{
  if (mTimerFPS)
  {
    onAnimationTimer();
  }
}

void pxWindowNative::runEventLoop()
{
  exitFlag = false;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#ifdef USE_DRAW_THREAD

  int fps = 60;// mTimerFPS;

  if( (draw_thread == 0) &&
      pthread_create(&draw_thread, NULL, &draw_func, (void *) &fps))
  {
    fprintf(stderr, "Error creating thread\n");
  }
#endif
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  eventLoop();
}

void pxWindowNative::exitEventLoop()
{
  exitFlag = true;

  DFB_CHECK (dfbBuffer->WakeUp(dfbBuffer)); // Unblock I/O

  printf("************************* %s\n",__PRETTY_FUNCTION__);// JUNK

#ifdef USE_DRAW_THREAD
  // wait for the draw thread to finish
  if(draw_thread && pthread_join(draw_thread, NULL))
  {
    fprintf(stderr, "Error joining thread\n");
    return;
  }
#endif

}

void pxWindowNative::animateAndRender()
{
  static double lastAnimationTime = pxMilliseconds();
  double currentAnimationTime = pxMilliseconds();

  drawFrame();

  double animationDelta = currentAnimationTime - lastAnimationTime;

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
  pxSurfaceNativeDesc d;

  d.dfb          = dfb;
  d.surface      = dfbSurface;
  d.windowWidth  = mLastWidth;
  d.windowHeight = mLastHeight;

  onDraw(&d);
}

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

void pxWindowNative::runEventLoopOnce()
{
   printf("************************* %s\n",__PRETTY_FUNCTION__);// JUNK
}

char* p2str(DFBSurfacePixelFormat fmt)
{
  switch(fmt)
  {
    case DSPF_UNKNOWN:    return (char*) "DSPF_UNKNOWN";
    case DSPF_ARGB1555:   return (char*) "DSPF_ARGB1555";
    case DSPF_RGB16:      return (char*) "DSPF_RGB16";
    case DSPF_RGB24:      return (char*) "DSPF_RGB24";
    case DSPF_RGB32:      return (char*) "DSPF_RGB32";
    case DSPF_ARGB:       return (char*) "DSPF_ARGB";
    case DSPF_A8:         return (char*) "DSPF_A8";
    case DSPF_YUY2:       return (char*) "DSPF_YUY2";
    case DSPF_RGB332:     return (char*) "DSPF_RGB332";
    case DSPF_UYVY:       return (char*) "DSPF_UYVY";
    case DSPF_I420:       return (char*) "DSPF_I420";
    case DSPF_YV12:       return (char*) "DSPF_YV12";
    case DSPF_LUT8:       return (char*) "DSPF_LUT8";
    case DSPF_ALUT44:     return (char*) "DSPF_ALUT44";
    case DSPF_AiRGB:      return (char*) "DSPF_AiRGB";
    case DSPF_A1:         return (char*) "DSPF_A1";
    case DSPF_NV12:       return (char*) "DSPF_NV12";
    case DSPF_NV16:       return (char*) "DSPF_NV16";
    case DSPF_ARGB2554:   return (char*) "DSPF_ARGB2554";
    case DSPF_ARGB4444:   return (char*) "DSPF_ARGB4444";
    case DSPF_RGBA4444:   return (char*) "DSPF_RGBA4444";
    case DSPF_NV21:       return (char*) "DSPF_NV21";
    case DSPF_AYUV:       return (char*) "DSPF_AYUV";
    case DSPF_A4:         return (char*) "DSPF_A4";
    case DSPF_ARGB1666:   return (char*) "DSPF_ARGB1666";
    case DSPF_ARGB6666:   return (char*) "DSPF_ARGB6666";
    case DSPF_RGB18:      return (char*) "DSPF_RGB18";
    case DSPF_LUT2:       return (char*) "DSPF_LUT2";
    case DSPF_RGB444:     return (char*) "DSPF_RGB444";
    case DSPF_RGB555:     return (char*) "DSPF_RGB555";
    case DSPF_BGR555:     return (char*) "DSPF_BGR555";
    case DSPF_RGBA5551:   return (char*) "DSPF_RGBA5551";
    case DSPF_YUV444P:    return (char*) "DSPF_YUV444P";
    case DSPF_ARGB8565:   return (char*) "DSPF_ARGB8565";
    case DSPF_AVYU:       return (char*) "DSPF_AVYU";
    case DSPF_VYU:        return (char*) "DSPF_VYU";
    case DSPF_A1_LSB:     return (char*) "DSPF_A1_LSB";
    case DSPF_YV16:       return (char*) "DSPF_YV16";
    case DSPF_ABGR:       return (char*) "DSPF_ABGR";
    case DSPF_RGBAF88871: return (char*) "DSPF_RGBAF88871";
    case DSPF_LUT4:       return (char*) "DSPF_LUT4";
    case DSPF_ALUT8:      return (char*) "DSPF_ALUT8";
    case DSPF_LUT1:       return (char*) "DSPF_LUT1";
  }//SWITCH

  return (char *) "NOT FOUND";
}





//###########################################################################

#ifdef USE_DRAW_THREAD

#ifndef USE_DRAW_ALARM

void mysleep_ms(int32_t ms)
{
    struct timespec res;

    res.tv_sec  = (ms / 1000);
    res.tv_nsec = (ms * 1000000) % 1000000000;

    clock_nanosleep(CLOCK_MONOTONIC, 0, &res, NULL);
}

void *draw_func(void *ptr)
{
  double fps_rate = ptr ? *((int *) ptr) : 60.0;
  double sleep_ms = (1.0/(float) fps_rate) * 1000.0;

  printf("\n\n\n SET fps = %f    %f ms\n\n\n", fps_rate, sleep_ms);

  static double lastFrame_ms = pxMilliseconds();

  while(!exitFlag)
  {
     onTimer(0);

     double now_ms  = pxMilliseconds();
     double elapsed = now_ms - lastFrame_ms;

     lastFrame_ms = now_ms;

     // schedule next timer event
    mysleep_ms( sleep_ms - elapsed );
  }

  return NULL;
}
//###########################################################################

#else // USE_DRAW_ALARM

timer_t fpsTimer;

void drawFrame(int signum)
{
    //fprintf(stderr, "Tick ###");
    onTimer(0);
}

void setupFpsTimer(int fps)
{
   struct itimerspec new_value, old_value;
   struct sigaction action;
   struct sigevent sevent;
   sigset_t set;
//   int signum;

  printf("\n\n\n ALARM - SET fps = %d\n\n\n", fps, sleep_ms);

   // SIGALRM for printing time
   memset(&action, 0, sizeof(struct sigaction));
   action.sa_handler = drawFrame;

   if (sigaction(SIGALRM, &action, NULL) == -1)
      perror ("sigaction");

   // for program completion
   memset (&sevent, 0, sizeof (struct sigevent));
   sevent.sigev_notify = SIGEV_SIGNAL;
   sevent.sigev_signo  = SIGRTMIN;

   if (timer_create(CLOCK_MONOTONIC, NULL, &fpsTimer) == -1)
      perror ("timer_create");

   const float ns_per_second = 1.0e9f; // nanoseconds per seconds
   float sleep_ns = (1.0f/(float) fps) * ns_per_second;

   new_value.it_interval.tv_sec  = 0;
   new_value.it_interval.tv_nsec = (long) sleep_ns; //###
   new_value.it_value.tv_sec     = 0;
   new_value.it_value.tv_nsec    = (long) sleep_ns; //###

   if (timer_settime( fpsTimer, 0, &new_value, &old_value) == -1)
      perror ("timer_settime");

   if (sigemptyset (&set) == -1)
      perror ("sigemptyset");

   if (sigaddset (&set, SIGRTMIN) == -1)
      perror ("sigaddset");

   if (sigprocmask (SIG_BLOCK, &set, NULL) == -1)
      perror ("sigprocmask");
}

void *draw_func(void *ptr)
{
  double fps_rate = ptr ? *((int *) ptr) : 60.0;
  double sleep_ms = (1.0/(float) fps_rate) * 1000.0;

  printf("\n\n\n SET fps = %f    %f ms\n\n\n", fps_rate, sleep_ms);

  setupFpsTimer(fps_rate);

  return NULL;
}
//###########################################################################

#endif //00

#endif // USE_DRAW_THREAD
