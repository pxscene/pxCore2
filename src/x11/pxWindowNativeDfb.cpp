// pxCore CopyRight 2005-2006 John Robinson
// Portable Framebuffer and Windowing Library
// pxWindowNativeDfb.cpp

#include "../pxCore.h"
#include "../pxWindow.h"
#include "../pxTimer.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define DFB_PX_CORE_FPS 30

static IDirectFB              *dfb;
static IDirectFBDisplayLayer  *layer;

static IDirectFBWindow        *window;
static IDirectFBSurface       *wsurface;
static IDirectFBEventBuffer   *wbuffer;
static IDirectFBEventBuffer   *ibuffer;

static IDirectFBInputDevice   *dfbKeyboard;
//static IDirectFBInputDevice   *dfbMouse;

bool exitFlag = false;

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

//start dfb callbacks

void onKeyboardDown(int key, unsigned long mods);
void onKeyboardUp(int key, unsigned long mods);
void onKeyboard(int key, unsigned long mods, bool down);

void onMouseMotion(int x, int y);
void onMouse(int button, int state, int x, int y);

static void reshape(int width, int height)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onSize(width, height);
  }
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
  //glutSwapBuffers(); // Flip ?
}



//#define EPRINTF(...) printf(__VA_ARGS__)
#define EPRINTF(f_, ...)

static void
ProcessWindowEvent(DFBWindowEvent * evt)
{
  if (evt->clazz == DFEC_WINDOW)
  {
    switch (evt->type)
    {
      case DWET_BUTTONDOWN:
        EPRINTF("\n DWET_BUTTONDOWN");

        // Send to MOUSE - BUTTON DOWN

        break;
      case DWET_BUTTONUP:
        EPRINTF("\n DWET_BUTTONUP");

        // Send to MOUSE - BUTTON UP

        break;
      case DWET_MOTION:
       // EPRINTF("\n DWET_MOTION");

        // Send to MOUSE - BUTTON MOTION

        break;

      case DWET_WHEEL:
        // TODO
        break;

      case DWET_KEYDOWN:
        EPRINTF("\n DWET_KEYDOWN");

        // Send to KEYBOARD - BUTTON DOWN

        break;
      case DWET_KEYUP:
        EPRINTF("\n DWET_KEYUP");

        // Send to KEYBOARD - BUTTON UP

        break;
      case DWET_POSITION:
        EPRINTF("\n DWET_POSITION - (%d,%d)",evt->x, evt->y);

        // Send to WINDOW - POSITION

        break;
      case DWET_POSITION_SIZE:
        EPRINTF("\n DWET_POSITION_SIZE - (%d,%d) WxH: %d x %d",evt->x, evt->y,  evt->w, evt->h);;

        // Send to WINDOW - POSITION + SIZE

        /* fall through */
      case DWET_SIZE:
        {
          int ww = 0, hh = 0;
          window->GetSize(window, &ww, &hh);

          EPRINTF("\n DWET_SIZE - WxH: %d x %d", ww, hh);

          reshape(ww, hh);
          // Send to WINDOW - SIZE
        }
        break;
      case DWET_CLOSE:
        EPRINTF("\n DWET_CLOSE");

        // Send to WINDOW - CLOSE

        break;
      case DWET_GOTFOCUS:

        // Send to WINDOW - GOT FOCUS

        break;
      case DWET_LOSTFOCUS:
        EPRINTF("\n DWET_LOSTFOCUS");

        // Send to WINDOW - LOST FOCUS

        break;
      case DWET_ENTER:

        // Send to WINDOW - ENTER

        break;
      case DWET_LEAVE:
        EPRINTF("\n DWET_LEAVE");

        // Send to WINDOW - LEAVE

        break;
      default:
        ;
    }
  }
}

// TODO ... probably better per window.
static int cursor_x =0;
static int cursor_y =0;

static void
ProcessInputEvent(DFBInputEvent *ievt)
{
  unsigned long mods = 0;

  if( ievt->modifiers == DIMM_SHIFT )   mods |= PX_MOD_SHIFT;
  if( ievt->modifiers == DIMM_CONTROL ) mods |= PX_MOD_CONTROL;
  if( ievt->modifiers == DIMM_META )    mods |= PX_MOD_ALT;

  printf("ProcessInputEvent:  ievt->modifiers = %d  >>  mods = 0x%08X\n", mods);

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

      onMouseMotion(cursor_x, cursor_y);
      break;

    case DIET_KEYPRESS:
      EPRINTF("\n DIET_KEYPRESS");

      onKeyboardDown(ievt->key_id, mods);
      break;

    case DIET_KEYRELEASE:
      EPRINTF("\n DIET_KEYRELEASE");

      onKeyboardUp(ievt->key_id, mods);
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

void eventLoop()
{
  while (!exitFlag)
  {
    DFBWindowEvent wEvent;
    DFBInputEvent  iEvent;

   // DFB_CHECK (wbuffer->WaitForEvent(wbuffer));

  //  dfb->WaitForEventWithTimeout, 0, 10); //ms

    // Get WINDOW events in case we need them
    while (wbuffer->GetEvent(wbuffer, DFB_EVENT(&wEvent)) == DFB_OK)
    {
      ProcessWindowEvent( &wEvent);

    }//WHILE - Window Events

    // Get INPUT events in case we need them
    while (wbuffer->GetEvent(ibuffer, DFB_EVENT(&iEvent)) == DFB_OK)
    {
      ProcessInputEvent( &iEvent);

    }//WHILE - Input Events

    // TODO:  A bit of a HACK !!!
    display();
    usleep(10);

  }//WHILE - LOOP
}


void onTimer(int v)
{
  display();
  // schedule next timer event
  //glutTimerFunc(16, onTimer, 0);
}

void onMouse(int button, int state, int x, int y)
{
  unsigned long flags = 0;

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
    else if(state == DIET_BUTTONRELEASE)
    {
      w->onMouseUp(x, y, flags);
    }
    else
    {
      w->onMouseMove(x, y);
    }
  }//FOR
}

void onMouseMotion(int x, int y)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onMouseMove(x, y);
  }
}

void onMousePassiveMotion(int x, int y)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onMouseMove(x, y);
  }
}

void onKeyboardDown(int key, unsigned long mods)
{
  onKeyboard(key, mods, true);
}

void onKeyboardUp(int key, unsigned long mods)
{
  onKeyboard(key, mods, false);
}

void onKeyboard(int key, unsigned long mods, bool down)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);

    if(down)
    {
      w->onKeyDown(key, mods);
    }
    else
    {
      w->onKeyUp(key, mods);
    }
  }
}

//end DFB callbacks

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

pxError displayRef::createDfbDisplay()
{
  if (mDisplay == NULL)
  {
    mDisplay = new dfbDisplay();
  }

  DirectFBInit( NULL, NULL );

  DirectFBCreate( &dfb );

  // Get the primary layer
  if (dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ))
  {
    dfb->Release( dfb );
    return 0;
  }

  if (layer->SetCooperativeLevel( layer, DLSCL_SHARED ))
  {
    layer->Release( layer );
    dfb->Release( dfb );
    return 0;
  }

  DFB_CHECK(dfb->GetInputDevice(dfb, DIDID_KEYBOARD, &dfbKeyboard));
  DFB_CHECK(dfbKeyboard->CreateEventBuffer(dfbKeyboard, &ibuffer));

//  if (dfb->SetCooperativeLevel( dfb, DFSCL_NORMAL ) ) // DFSCL_NORMAL   DFSCL_FULLSCREEN
//  {
//    dfb->Release( dfb );
//    return 0;
//  }


  //callbacks
  // XXX: These have no affect glutGetWindow() == 0
//#if 0
//  glutDisplayFunc(display);
//  glutReshapeFunc(reshape);
//  glutMouseFunc(onMouse);
//  glutMotionFunc(onMouseMotion);
//  glutPassiveMotionFunc(onMousePassiveMotion);
//  glutKeyboardFunc(onKeyboard);
//#endif
  return PX_OK;
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

void pxWindowNative::createDfbWindow(int left, int top, int width, int height)
{
  DFBWindowDescription wd;

  // Create a window to house the video layer
  wd.flags  = (DFBWindowDescriptionFlags) (DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT);
  wd.posx   = left;
  wd.posy   = top;
  wd.width  = width;
  wd.height = height;

  layer->CreateWindow( layer, &wd, &window );

  window->GetSurface( window, &wsurface );

  wsurface->Clear( wsurface, 0x00, 0x80, 0x00,  0xff );

  wsurface->Flip( wsurface, NULL, (DFBSurfaceFlipFlags) 0 );

  window->SetOpacity( window, 0xff );

  // Select events
//  window->DisableEvents( window, DWET_ALL );
//  window->EnableEvents( window, DWET_BUTTONDOWN | DWET_KEYDOWN |
//                        DWET_WHEEL  | DWET_POSITION | DWET_SIZE );

  // Create Eventbuffer
  DFB_ERROR(window->CreateEventBuffer(window, &wbuffer));
  DFB_ERROR(window->EnableEvents(window, DWET_ALL));
}

void pxWindowNative::cleanupDfbWindow()
{
  if(wbuffer)
  {
    wbuffer->Release(wbuffer);
  }

  if(wsurface)
  {
    wsurface->Release(wsurface);
  }

  if(window)
  {
    window->Release(window);
  }
}

pxError pxWindow::init(int left, int top, int width, int height)
{
  dfbDisplay* display = mDisplayRef.getDisplay();

  if (display == NULL)
  {
    cout << "Error initializing display\n" << endl;
    return PX_FAIL;
  }
  else
  {
    mLastWidth  = width;
    mLastHeight = height;
    mResizeFlag = true;

    createDfbWindow(left,top,width,height);

//    // XXX: Need to register callbacks after window is created^M
//    glutReshapeFunc(reshape);
//    //    glutDisplayFunc(display);
//    //    glutReshapeFunc(reshape);
//    glutMouseFunc(onMouse);
//    glutMotionFunc(onMouseMotion);
//    glutPassiveMotionFunc(onMousePassiveMotion);
//    glutKeyboardFunc(onKeyboard);

    registerWindow(this);

    this->onCreate();

    reshape(width,height);
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
  drawFrame();
}

bool pxWindow::visibility()
{
  return mVisible;
}

void pxWindow::setVisibility(bool visible)
{
  mVisible = visible;
  if (mVisible)
  {
      window->SetOpacity( window, 0xff );
//    dfbShowWindow();
  }
  else
  {
    window->SetOpacity( window, 0x0 );
//    dfbHideWindow();
  }
}

pxError pxWindow::setAnimationFPS(long fps)
{
  mTimerFPS = fps;
  mLastAnimationTime = pxMilliseconds();
  return PX_OK;
}

void pxWindow::setTitle(char* title)
{
  //glutSetWindowTitle(title);
}

pxError pxWindow::beginNativeDrawing(pxSurfaceNative& s)
{
  //TODO

  return PX_OK;
}

pxError pxWindow::endNativeDrawing(pxSurfaceNative& s)
{
  //TODO

  return PX_OK;
}

// pxWindowNative

void pxWindowNative::onAnimationTimerInternal()
{
  if (mTimerFPS) onAnimationTimer();
}

void pxWindowNative::runEventLoop()
{
  exitFlag = false;

//    glutTimerFunc(32, onTimer, 0); >> display()

  eventLoop();
}

void pxWindowNative::exitEventLoop()
{
  exitFlag = true;
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
  d.surface      = wsurface;
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
